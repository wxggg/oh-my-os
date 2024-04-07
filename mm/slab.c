#include <memory.h>
#include <slab.h>
#include <stdio.h>
#include <assert.h>
#include <vmalloc.h>
#include <bitops.h>
#include <atomic.h>

#define MODULE "slab"
#define MODULE_DEBUG 0

static inline void *index_to_obj(struct kmem_cache *cache, struct page *page,
				 unsigned int idx)
{
	return page->s_mem + cache->size * idx;
}

static inline unsigned long obj_to_index(struct kmem_cache *cache,
					 struct page *page, void *obj)
{
	unsigned long offset = obj - page->s_mem;
	return offset / cache->size;
}

static inline int slab_page_init(struct kmem_cache *cache, struct page *page)
{
	unsigned short i, total;

	page->s_mem = page_address(page);
	if (!page->s_mem)
		return -ENOMEM;

	total = PAGE_SIZE / cache->size;

	page->freelist = (unsigned char *)page->s_mem + PAGE_SIZE - total;
	page->total = (PAGE_SIZE - total) / cache->size;
	page->active = 0;
	page->slab_cache = cache;

	for (i = 0; i < total; i++)
		page->freelist[i] = i;

	set_bit(PAGE_SLAB, &page->flags);
	return 0;
}

static inline void slab_page_deinit(struct page *page)
{
	page->freelist = NULL;
	page->total = 0;
	page->active = 0;
	page->slab_cache = 0;
	clear_bit(PAGE_SLAB, &page->flags);
}

static inline void *alloc_block(struct kmem_cache *cache, struct page *page)
{
	void *obj;
	unsigned long index;

	assert(page->active < page->total);

	index = page->freelist[page->active++];
	obj = index_to_obj(cache, page, index);

	if (page->active == page->total) {
		list_remove(&page->node);
		list_insert(&cache->slabs_full, &page->node);
	}

	return obj;
}

static inline void free_block(struct kmem_cache *cache, struct page *page, void *obj)
{
	unsigned long index;

	index = obj_to_index(cache, page, obj);

	assert(page->active > 0);
	page->freelist[--page->active] = index;

	if (page->active == 0) {
		list_remove(&page->node);
		slab_page_deinit(page);
		free_pages(page);
	}
}

void *kmem_cache_alloc(struct kmem_cache *cache)
{
	struct page *page;
	struct list_node *node;
	void *ptr;

	spin_lock(&cache->lock);
	if (list_empty(&cache->slabs_partial)) {
		page = alloc_page(GFP_NORMAL);
		if (slab_page_init(cache, page))
			return NULL;
		list_insert(&cache->slabs_partial, &page->node);
	} else {
		node = list_next(&cache->slabs_partial);
		page = container_of(node, struct page, node);
	}

	assert(page);
	ptr = alloc_block(cache, page);
	spin_unlock(&cache->lock);

	return ptr;
}

void kmem_cache_free(struct kmem_cache *cache, void *obj)
{
	struct page *page;

	assert(!is_vmalloc_addr((uintptr_t)obj));

	page = virt_to_page((uintptr_t)obj);
	assert(page && page->slab_cache == cache && !test_bit(PAGE_HIGHMEM, &page->flags));

	spin_lock(&cache->lock);
	free_block(cache, page, obj);
	spin_unlock(&cache->lock);
}

int kmem_cache_create(struct kmem_cache *cache, size_t size)
{
	if (size > PAGE_SIZE)
		return -EINVAL;

	list_init(&cache->slabs_full);
	list_init(&cache->slabs_partial);
	cache->size = size;
	spinlock_init(&cache->lock);
	return 0;
}
