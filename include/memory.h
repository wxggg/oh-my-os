#pragma once

#include <asm-generic/mmu.h>
#include <types.h>
#include <list.h>
#include <stdlib.h>
#include <slab.h>
#include <bitops.h>

/**
 * physical memory layout
 *
 * +------------------------+ 0xffffffff (4GB)
 * |                        |
 * |                        |
 * +------------------------+
 * |      ioapic            |
 * +------------------------+ 0xfec00000
 * |                        |
 * +------------------------+
 * |      video memory      |
 * +------------------------+ 0xfd000000
 * |                        |
 * +------------------------+ 0xf8000000 (3GB + 896MB)
 * |      reserved          |
 * +------------------------+ ram end
 * |                        |
 * |                        |
 * |      high mem          |
 * |                        |
 * |                        |
 * +------------------------+-----------+ 0x38000000 (896MB)
 * |                        |           |
 * |                        |           |
 * |      linear            |  linear   |
 * |                        |           |
 * |                        |           |
 * +------------------------+           + high mem start
 * |                        |           | kernel end
 * |      kernel            |   map     |
 * +------------------------+-----------+ 0x0
 */

/**
 * virtual memory layout
 *
 * +------------------------+ 0xffffffff (4GB)
 * |       ioremap address  | 128MB
 * +------------------------+ -----------+ 0xf8000000 (3GB + 896MB)
 * |                        |            |
 * |       linear           |            |
 * |                        |   linear   |
 * +------------------------+            + high mem start
 * |                        |            | kernel end
 * |       kernel           |            |
 * +------------------------+------------+ 0xc0000000 (3GB)
 * |                        |
 * |                        |
 * |      vmalloc           |
 * |                        |
 * |                        |
 * +------------------------+ 0x00001000 (4KB)
 * |      reserved          |
 * +------------------------+ NULL
 */

#define PHYS_LINEAR_END 0x38000000
#define PHYS_HIGHMEM_START 0x38000000
#define PHYS_HIGHMEM_END 0xf8000000
#define PHYS_IOMEM_START 0xf8000000
#define IOMEM_SIZE 0x08000000

#define LINEAR_MAP_SIZE (PHYS_LINEAR_END)

#define VMALLOC_START PAGE_SIZE
#define VMALLOC_END KERNEL_VIRT_BASE
#define VIRT_LINEAR_END (KERNEL_VIRT_BASE + LINEAR_MAP_SIZE)
#define VIRT_IOREMAP_BASE VIRT_LINEAR_END

typedef unsigned int gfp_t;

#define GFP_HIGHMEM 0x01u
#define GFP_LINEAR 0x02u

#define GFP_NORMAL GFP_LINEAR
#define GFP_KERNEL GFP_NORMAL

/*
 * page structure
 *
 * @flags: page flags
 * @order: page in which order
 * @node: in free list
 * @free: true if can be allocated
 */
struct page {
	unsigned long flags;
	unsigned long order;
	struct list_node node;

	/* slab allocator */
	void *s_mem;
	unsigned char *freelist;
	unsigned short active;
	unsigned short total;
	struct kmem_cache *slab_cache;
};

#define in_range(x, start, length)           \
	(uintptr_t) x >= (uintptr_t)start && \
		(uintptr_t)x < ((uintptr_t)start + (size_t)length)

extern unsigned long kernel_start_pfn, kernel_end_pfn;
extern unsigned long highmem_start_pfn, highmem_end_pfn;
extern unsigned long vmalloc_start_pfn, vmalloc_end_pfn;

struct mm_context *memory_init(void);

/* page flag */
#define PAGE_VALID BIT(0)
#define PAGE_FREE BIT(1)
#define PAGE_HIGHMEM BIT(2)
#define PAGE_SLAB BIT(3)

#define pde_index(x) (((x) >> 22) & 0x3ff)
#define pte_index(x) (((x) >> 12) & 0x3ff)

#define page_base(x) ((x) & ~0xfff)
#define page_offset(x) ((x)&0xfff)

#define round_up_page(x) round_up((uintptr_t)(x), PAGE_SIZE)
#define round_down_page(x) round_down((uintptr_t)(x), PAGE_SIZE)

void page_init(void);

/* page table */
void set_pde(unsigned long *pde, unsigned long pa, uint32_t flag);
void page_map(unsigned long *pgdir, unsigned long va, unsigned long pa,
	      size_t size, uint32_t flag);
void page_unmap(unsigned long *pgdir, unsigned long va, size_t size);
void page_table_dump(unsigned long *pgdir, unsigned long va, size_t size);
void enable_paging(unsigned long cr3);
void start_paging(uint32_t *pgdir);
void *page_address(struct page *page);

#define virt_to_phys(x) ((uintptr_t)(x)-KERNEL_VIRT_BASE)
#define phys_to_virt(x) ((uintptr_t)(x) + KERNEL_VIRT_BASE)
#define phys_to_pfn(x) (((uintptr_t)x) >> PAGE_SHIFT)

unsigned long page_to_pfn(struct page *page);
struct page *pfn_to_page(unsigned long pfn);

static inline unsigned long page_to_phys(struct page *page)
{
	return (unsigned long)PAGE_SIZE * page_to_pfn(page);
}

static inline struct page *phys_to_page(unsigned long phys)
{
	return pfn_to_page(phys_to_pfn(phys));
}

static inline struct page *virt_to_page(unsigned long virt)
{
	return phys_to_page(virt_to_phys(virt));
}

static inline unsigned long page_to_virt(struct page *page)
{
	return phys_to_virt(page_to_phys(page));
}

void add_free_pages(unsigned long start_pfn, unsigned long end_pfn);
struct page *alloc_pages(gfp_t gfp_mask, unsigned int order);
void free_pages(struct page *page);

#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)

void kernel_map(unsigned long kva, unsigned long pa, size_t size,
		uint32_t flag);
void kernel_unmap(unsigned long va, size_t size);
void kernel_page_table_dump(unsigned long va, size_t size);

int page_init_late(void);
