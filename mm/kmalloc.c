#include <types.h>
#include <memory.h>
#include <kernel.h>
#include <assert.h>
#include <log2.h>
#include <stdio.h>
#include <fs.h>
#include <kernel.h>

struct block {
	unsigned int order;
	struct list_node node;
	bool free;
};

#define MIN_SIZE 8
#define MAX_ORDER 12
#define MAX_SIZE (MIN_SIZE << MAX_ORDER)

static unsigned char block_buffer[MAX_SIZE];
static struct block blocks[1 << MAX_ORDER];
static struct list_node free_lists[MAX_ORDER + 1];

#define KMEM_CACHE_MAX_ORDER 7
#define KMEM_CACHE_MAX_SIZE (MIN_SIZE << KMEM_CACHE_MAX_ORDER)
static struct kmem_cache kmalloc_cache[KMEM_CACHE_MAX_ORDER + 1];
static bool init_kmem_cache = false;

static inline bool is_early_block_addr(unsigned long addr)
{
	unsigned long base = (unsigned long)block_buffer;
	return addr >= base && addr <= (base + MAX_SIZE);
}

static inline unsigned long block_to_index(struct block *block)
{
	return block - blocks;
}

static inline struct block *block_buddy(struct block *block)
{
	return &blocks[block_to_index(block) ^ (1 << block->order)];
}

static inline unsigned long index_to_addr(unsigned long index)
{
	return (unsigned long)block_buffer + index * MIN_SIZE;
}

static inline unsigned long addr_to_index(unsigned long addr)
{
	return (addr - (unsigned long)block_buffer) / MIN_SIZE;
}

static struct block *alloc_blocks(unsigned int order)
{
	struct list_node *node;
	struct block *block, *buddy;
	unsigned int i;

	assert(order <= MAX_ORDER, "too large order:", dec(order));

	for (i = order; i <= MAX_ORDER; i++) {
		if (!list_empty(&free_lists[i])) {
			node = list_next(&free_lists[i]);
			list_remove(node);
			block = container_of(node, struct block, node);
			assert(i == block->order);

			while (block->order > order) {
				block->order--;
				buddy = block_buddy(block);
				buddy->order = block->order;
				buddy->free = true;
				list_insert(&free_lists[buddy->order],
					    &buddy->node);
			}

			block->free = false;
			return block;
		}
	}

	return NULL;
}

static void free_blocks(struct block *block)
{
	struct block *buddy;

	while (block->order < MAX_ORDER) {
		buddy = block_buddy(block);
		if (!buddy->free)
			break;

		list_remove(&buddy->node);
		if (buddy < block)
			block = buddy;

		block->order++;
	}

	block->free = true;
	list_insert(&free_lists[block->order], &block->node);
}

void *kmalloc_early(size_t size)
{
	struct block *block;
	unsigned int order = ilog2_roundup(size / MIN_SIZE);

	block = alloc_blocks(order);
	if (!block)
		return NULL;

	return (void *)index_to_addr(block_to_index(block));
}

void kfree_early(void *ptr)
{
	struct block *block;
	unsigned long offset = (unsigned char *)ptr - block_buffer;
	unsigned int index;

	assert(offset < MAX_SIZE && (offset & (MIN_SIZE - 1)) == 0);

	index = addr_to_index((unsigned long)ptr);
	block = &blocks[index];
	free_blocks(block);
}

void kmalloc_early_init(void)
{
	unsigned int i;
	struct block *block;

	for (i = 0; i <= MAX_ORDER; i++)
		list_init(&free_lists[i]);

	block = &blocks[0];
	block->order = MAX_ORDER;
	block->free = true;
	list_insert(&free_lists[MAX_ORDER], &block->node);
	pr_info("kmalloc early init success");
}

void *kmalloc(size_t size)
{
	unsigned int order;
	struct page *page;

	if (!init_kmem_cache)
		return kmalloc_early(size);

	if (size <= KMEM_CACHE_MAX_SIZE) {
		order = ilog2_roundup(round_up(size, MIN_SIZE) / MIN_SIZE);
		assert(order <= KMEM_CACHE_MAX_ORDER);
		return kmem_cache_alloc(&kmalloc_cache[order]);
	}

	order = ilog2_roundup(round_up_page(size) / PAGE_SIZE);
	page = alloc_pages(GFP_NORMAL, order);
	if (!page)
		return NULL;

	return (void *)page_to_virt(page);
}

void kfree(void *ptr)
{
	struct page *page;

	if (is_early_block_addr((unsigned long)ptr)) {
		kfree_early(ptr);
		return;
	}

	page = virt_to_page((unsigned long)ptr);
	if (is_bit_set(page->flags, PAGE_SLAB)) {
		assert(page->slab_cache);
		kmem_cache_free(page->slab_cache, ptr);
		return;
	} else {
		free_pages(page);
	}
}

void kmalloc_init(void)
{
	unsigned int i;

	for (i = 0; i <= KMEM_CACHE_MAX_ORDER; i++)
		kmem_cache_create(&kmalloc_cache[i], MIN_SIZE << i);

	init_kmem_cache = true;
}

static int dump_kmalloc_early(string *s)
{
	u32 i, count;

	ksappend_str(s, "init buffer: <");
	ksappend_hex(s, (uintptr_t)block_buffer);
	ksappend_str(s, ", ");
	ksappend_hex(s, (uintptr_t)block_buffer + MAX_SIZE);
	ksappend_str(s, "> \n");

	for (i = 0; i <= MAX_ORDER; i++) {
		count = list_size(&free_lists[i]);
		if (count > 0) {
			ksappend_str(s, "size-");
			ksappend_int(s, MIN_SIZE << i);
			ksappend_str(s, ": ");
			ksappend_int(s, count);
			ksappend_str(s, "\n");
		}
	}

	return 0;
}

static int dump_kmalloc(string *s)
{
	unsigned int i;
	struct kmem_cache *kcache;
	struct list_node *node;
	struct page *page;

	ksappend_kv(s, "max_size:", KMEM_CACHE_MAX_SIZE);
	ksappend_str(s, "\n");
	ksappend_kv(s, "max_order:", KMEM_CACHE_MAX_ORDER);
	ksappend_str(s, "\n");

	for (i = 0; i <= KMEM_CACHE_MAX_ORDER; i++) {
		kcache = &kmalloc_cache[i];

		if (list_empty(&kcache->slabs_full) &&
		    list_empty(&kcache->slabs_partial))
			continue;

		ksappend_kv(s, "size:", kcache->size);
		ksappend_kv(s, " slabs_full:", list_size(&kcache->slabs_full));
		ksappend_kv(s, " slabs_partial:", list_size(&kcache->slabs_partial));

		for (node = kcache->slabs_partial.next;
		     node != &kcache->slabs_partial; node = node->next) {
			page = container_of(node, struct page, node);
			ksappend_kv(s, " ", page->active);
		}

		ksappend_str(s, "\n");
	}

	return 0;
}

static struct file_operations dump_kmalloc_early_fops = {
	.read = dump_kmalloc_early,
};

static struct file_operations dump_kmalloc_fops = {
	.read = dump_kmalloc,
};

int kmalloc_init_late(void)
{
	struct file *file;

	create_file("kmalloc", &dump_kmalloc_fops, sys, &file);
	create_file("kmalloc-early", &dump_kmalloc_early_fops, sys, &file);
	return 0;
}
