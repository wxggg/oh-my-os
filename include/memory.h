#pragma once

#include <asm-generic/mmu.h>
#include <types.h>
#include <list.h>
#include <stdlib.h>

/**
 * physical memory layout
 *
 * +------------------------+ 0xffffffff (4GB)
 * |      io mem            | 128MB
 * +------------------------+ 0xf8000000 (3GB + 896MB)
 * |      reserved          |
 * +------------------------+ ram end
 * |                        |
 * |                        |
 * |      vmalloc           |
 * |                        |
 * |                        |
 * +------------------------+-----------+ 0x38000000 (896MB)
 * |                        |           |
 * |                        |           |
 * |      high mem          |  linear   |
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
 * |       high mem         |            |
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

#define PHYS_HIGHMEM_END 0x38000000
#define PHYS_VMALLOC_START 0x38000000
#define PHYS_VMALLOC_END 0xf8000000
#define PHYS_IOMEM_START 0xf8000000

#define LINEAR_MAP_SIZE (PHYS_HIGHMEM_END)

#define VMALLOC_START PAGE_SIZE
#define VMALLOC_END KERNEL_VIRT_BASE
#define VIRT_HIGHMEM_END (KERNEL_VIRT_BASE + LINEAR_MAP_SIZE)
#define VIRT_IOREMAP_BASE VIRT_HIGHMEM_END

typedef unsigned int gfp_t;

#define GFP_HIGHMEM 0x01u
#define GFP_NORMAL 0x02u

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
	bool free;
};

#define in_range(x, start, length) \
	(uintptr_t)x >= (uintptr_t)start && \
	(uintptr_t)x < ((uintptr_t)start + (size_t)length)

extern unsigned long kernel_start_pfn, kernel_end_pfn;
extern unsigned long highmem_start_pfn, highmem_end_pfn;
extern unsigned long vmalloc_start_pfn, vmalloc_end_pfn;

void memory_init(void);

/* page flag */
#define PAGE_VALID	(1 << 0)
#define PAGE_HIGHMEM	(1 << 1)

#define pde_index(x) (((x) >> 22) & 0x3ff)
#define pte_index(x) (((x) >> 12) & 0x3ff)

#define page_base(x) ((x) & ~0xfff)
#define page_offset(x) ((x) & 0xfff)

#define page_set_valid(page) ((page)->flags |= PAGE_VALID)
#define page_is_valid(page) ((page)->flags & PAGE_VALID)
#define page_set_highmem(page) ((page)->flags |= PAGE_HIGHMEM)
#define page_is_highmem(page) ((page)->flags & PAGE_HIGHMEM)

#define round_up_page(x)	round_up((uintptr_t)(x), PAGE_SIZE)
#define round_down_page(x)	round_down((uintptr_t)(x), PAGE_SIZE)

void page_init(void);

/* page table */
void set_pde(unsigned long* pde, unsigned long pa, uint32_t flag);
void page_map(unsigned long *pgdir, unsigned long va, unsigned long pa, size_t size, uint32_t flag);
void page_unmap(unsigned long *pgdir, unsigned long va, size_t size);
void page_table_dump(unsigned long *pgdir, unsigned long va, size_t size);
void enable_paging(unsigned long cr3);

#define virt_to_phys(x) ((uintptr_t)(x) - KERNEL_VIRT_BASE)
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

void add_free_pages(unsigned long start_pfn, unsigned long end_pfn);
struct page *alloc_pages(gfp_t gfp_mask, unsigned int order);
void free_pages(struct page *page);

#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)

void kernel_map(unsigned long kva, unsigned long pa, size_t size, uint32_t flag);
void kernel_unmap(unsigned long va, size_t size);
void kernel_page_table_dump(unsigned long va, size_t size);
