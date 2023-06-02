#include <types.h>
#include <rb_tree.h>
#include <assert.h>
#include <memory.h>
#include <list.h>
#include <log2.h>
#include <bitops.h>
#include <kmalloc.h>
#include <error.h>

#define VMA_DEBUG

struct vmap_area {
	unsigned long start;
	unsigned long end;
	bool free;

	struct list_node node;
	struct list_node free_node;
};

#define MAX_VMA_ORDER 20

static struct rb_tree *vma_tree;
struct list_node vma_list;
struct list_node free_vma_lists[MAX_VMA_ORDER + 1];

static inline unsigned long vma_length(struct vmap_area *vma)
{
	return vma->end - vma->start + 1;
}

static inline struct vmap_area *vma_prev(struct vmap_area *vma)
{
	struct list_node *node = vma->node.prev;

	if (node == &vma_list)
		return NULL;

	return container_of(node, struct vmap_area, node);
}

static inline struct vmap_area *vma_next(struct vmap_area *vma)
{
	struct list_node *node = vma->node.next;

	if (node == &vma_list)
		return NULL;

	return container_of(node, struct vmap_area, node);
}

static void free_vma(struct vmap_area *vma)
{
	struct vmap_area *prev, *next;

	pr_info("vma:", hex(vma));
	assert(vma);

	while ((prev = vma_prev(vma)) && prev->free) {
		assert((prev->end + 1) == vma->start, "vma is not adjacent, ",
			range(prev->start, prev->end), ", ", range(vma->start, vma->end));

		vma->start = prev->start;
		list_remove(&prev->node);
		list_remove(&prev->free_node);
		kfree(prev);
	}

	while ((next = vma_next(vma)) && next->free) {
		assert((next->end + 1) == vma->start, "vma is not adjacent, ",
			range(next->start, next->end), ", ", range(vma->start, vma->end));

		vma->end = next->end;
		list_remove(&next->node);
		list_remove(&next->free_node);
		kfree(next);
	}

	vma->free = true;
}

struct vmap_area *alloc_vma(unsigned long len)
{
	struct vmap_area *vma, *vma_buddy;
	struct list_node *node;
	unsigned long order, buddy_order;
	unsigned long i;

	order = ilog2_roundup(len >> PAGE_SHIFT);

	for (i = order; i <= MAX_VMA_ORDER; i++) {
		if (!list_empty(&free_vma_lists[i])) {
			node = list_next(&free_vma_lists[i]);
			vma = container_of(node, struct vmap_area, free_node);

			assert((vma->end - vma->start) >= len);

			if (vma_length(vma) >= len + PAGE_SIZE) {
				vma_buddy = kmalloc(sizeof(*vma_buddy));
				if (!vma_buddy)
					return NULL;

				vma_buddy->end = vma->end;
				vma->end = vma->start + len - 1;
				vma_buddy->start = vma->end + 1;

				buddy_order = ilog2(vma_length(vma_buddy) >> PAGE_SHIFT);
				assert(buddy_order <= MAX_VMA_ORDER);

				list_insert(&vma->node, &vma_buddy->node);
				list_insert(&free_vma_lists[buddy_order], &vma_buddy->free_node);
			}

			vma->free = false;
			rb_tree_insert(vma_tree, vma->start, vma->end, vma);
			return vma;
		}
	}

	return NULL;
}

void *vmalloc(size_t size)
{
	struct vmap_area *vma;
	struct page **pages;
	unsigned long i, nr_pages;
	unsigned vm_start;

	warn_on(size < PAGE_SIZE, " allocate size=", dec(size), " < PAGE_SIZE");

	size = round_up_page(size);
	nr_pages = size >> PAGE_SHIFT;

	vma = alloc_vma(size);
	if (!vma) {
		pr_err("alloc_vma failed");
		return NULL;
	}

	pages = kmalloc(sizeof(pages) * nr_pages);
	if (!pages)
		goto err_free_vma;

	for (i = 0; i < nr_pages; i++) {
		pages[i] = alloc_page();
		if (!pages[i]) {
			pr_err("alloc_page failed");
			goto err_free_pages;
		}
	}

	vm_start = vma->start;
	for (i = 0; i < nr_pages; i++) {
		kernel_map(vm_start, page_to_phys(pages[i]), PAGE_SIZE, PTE_W);
		vm_start += PAGE_SIZE;
	}

	return (void *)vma->start;

err_free_pages:
	while (i--) {
		free_pages(pages[i]);
	}

	kfree(pages);
err_free_vma:
	free_vma(vma);
	return NULL;
}

int vmalloc_init(void)
{
	int order;
	struct vmap_area *vma;

	for (order = 0; order <= MAX_VMA_ORDER; order++)
		list_init(&free_vma_lists[order]);

	list_init(&vma_list);

	vma = kmalloc(sizeof(*vma));
	if (!vma)
		return -ENOMEM;

	vma->start = 0x10000000;
	vma->end = 0xafffffff;
	vma->free = true;

	order = ilog2(vma_length(vma) >> PAGE_SHIFT);
	list_insert(&vma_list, &vma->node);
	list_insert(&free_vma_lists[order], &vma->free_node);
	return 0;
}
