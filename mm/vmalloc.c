#include <types.h>
#include <rb_tree.h>
#include <assert.h>
#include <memory.h>
#include <list.h>
#include <log2.h>
#include <bitops.h>
#include <kmalloc.h>
#include <error.h>
#include <mm.h>

#define VMA_DEBUG

#define MAX_VMA_ORDER 20

static struct rb_tree *vma_tree;
struct list_node vma_list;
struct list_node free_vma_lists[MAX_VMA_ORDER + 1];

static inline unsigned long vma_length(struct vm_area *vma)
{
	return vma->end - vma->start;
}

static inline struct vm_area *vma_prev(struct vm_area *vma)
{
	struct list_node *node = vma->node.prev;

	if (node == &vma_list)
		return NULL;

	return container_of(node, struct vm_area, node);
}

static inline struct vm_area *vma_next(struct vm_area *vma)
{
	struct list_node *node = vma->node.next;

	if (node == &vma_list)
		return NULL;

	return container_of(node, struct vm_area, node);
}

static void free_vma(struct vm_area *vma)
{
	struct vm_area *prev, *next;

	assert(vma);

	rb_tree_remove(vma_tree, vma->start);

	while ((prev = vma_prev(vma)) && prev->free) {
		assert((prev->end) == vma->start, "vma is not adjacent, ",
			range(prev->start, prev->end), ", ", range(vma->start, vma->end));

		vma->start = prev->start;
		list_remove(&prev->node);
		list_remove(&prev->free_node);
		kfree(prev);
	}

	while ((next = vma_next(vma)) && next->free) {
		assert((next->end) == vma->start, "vma is not adjacent, ",
			range(next->start, next->end), ", ", range(vma->start, vma->end));

		vma->end = next->end;
		list_remove(&next->node);
		list_remove(&next->free_node);
		kfree(next);
	}

	vma->free = true;
}

struct vm_area *alloc_vma(unsigned long len)
{
	struct vm_area *vma, *vma_buddy;
	struct list_node *node;
	unsigned long order, buddy_order;
	unsigned long i;

	order = ilog2_roundup(len >> PAGE_SHIFT);

	for (i = order; i <= MAX_VMA_ORDER; i++) {
		if (!list_empty(&free_vma_lists[i])) {
			node = list_next(&free_vma_lists[i]);
			vma = container_of(node, struct vm_area, free_node);

			assert((vma->end - vma->start) >= len);

			if (vma_length(vma) >= len + PAGE_SIZE) {
				vma_buddy = kmalloc(sizeof(*vma_buddy));
				if (!vma_buddy)
					return NULL;

				vma_buddy->end = vma->end;
				vma->end = vma->start + len;
				vma_buddy->start = vma->end;

				buddy_order = ilog2(vma_length(vma_buddy) >> PAGE_SHIFT);
				assert(buddy_order <= MAX_VMA_ORDER);

				list_insert(&vma->node, &vma_buddy->node);
				list_insert(&free_vma_lists[buddy_order], &vma_buddy->free_node);
			}

			vma->free = false;
			rb_tree_insert(vma_tree, vma->start, vma->end - 1, vma);
			return vma;
		}
	}

	return NULL;
}

void *__vmalloc(gfp_t gfp_mask, size_t size)
{
	struct vm_area *vma;
	unsigned long i;
	unsigned vm_start;

	warn_on(size < PAGE_SIZE, " allocate size=", dec(size), " < PAGE_SIZE");

	size = round_up_page(size);

	vma = alloc_vma(size);
	if (!vma) {
		pr_err("alloc_vma failed");
		return NULL;
	}

	vma->nr_pages = size >> PAGE_SHIFT;

	vma->pages = kmalloc(sizeof(vma->pages) * vma->nr_pages);
	if (!vma->pages)
		goto err_free_vma;

	for (i = 0; i < vma->nr_pages; i++) {
		vma->pages[i] = alloc_page(gfp_mask);
		if (!vma->pages[i]) {
			pr_err("alloc_page failed");
			goto err_free_pages;
		}
	}

	vm_start = vma->start;
	for (i = 0; i < vma->nr_pages; i++) {
		kernel_map(vm_start, page_to_phys(vma->pages[i]), PAGE_SIZE, PTE_W);
		vm_start += PAGE_SIZE;
	}

	return (void *)vma->start;

err_free_pages:
	while (i--) {
		free_pages(vma->pages[i]);
	}

	kfree(vma->pages);
err_free_vma:
	free_vma(vma);
	return NULL;
}

void *vmalloc(size_t size)
{
	void *ptr;

	ptr = __vmalloc(GFP_KERNEL, size);
	if (ptr)
		return ptr;

	return __vmalloc(GFP_HIGHMEM, size);
}

void vfree(void *addr)
{
	struct vm_area *vma;
	struct rb_node *node;
	unsigned long vaddr = (uintptr_t)addr;
	unsigned int i;

	if (!addr)
		return;

	assert(vaddr >= VMALLOC_START && vaddr < VMALLOC_END,
	       "invalid addr:", hex(addr));

	node = rb_tree_search(vma_tree, vaddr);
	assert(node);

	vma = rb_node_value(node);
	assert(vma);

	for (i = 0; i < vma->nr_pages; i++)
		free_pages(vma->pages[i]);

	kfree(vma->pages);
	vma->pages = NULL;
	vma->nr_pages = 0;

	free_vma(vma);
}

int vmalloc_init(void)
{
	int order;
	struct vm_area *vma;

	for (order = 0; order <= MAX_VMA_ORDER; order++)
		list_init(&free_vma_lists[order]);

	list_init(&vma_list);

	vma = kmalloc(sizeof(*vma));
	if (!vma)
		return -ENOMEM;

	vma->start = VMALLOC_START;
	vma->end = VMALLOC_END;
	vma->free = true;

	order = ilog2(vma_length(vma) >> PAGE_SHIFT);
	list_insert(&vma_list, &vma->node);
	list_insert(&free_vma_lists[order], &vma->free_node);

	vma_tree = rb_tree_create();
	assert_notrace(vma_tree);

	return 0;
}
