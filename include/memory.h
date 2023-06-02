#include <asm-generic/mmu.h>
#include <types.h>
#include <list.h>
#include <stdlib.h>

struct page {
	unsigned long flags;
	unsigned long order;
	struct list_node node;
	bool free;
};

#define in_range(x, start, length) \
	(uintptr_t)x >= (uintptr_t)start && \
	(uintptr_t)x < ((uintptr_t)start + (size_t)length)

void memory_init(void);

/* page */

#define PAGE_RESERVED	(1 << 0)
#define PAGE_AVAILABLE	(1 << 1)

#define pde_index(x) (((x) >> 22) & 0x3ff)
#define pte_index(x) (((x) >> 12) & 0x3ff)

#define page_base(x) ((x) & ~0xfff)
#define page_offset(x) ((x) & 0xfff)

#define page_set_reserved(page) (page->flags |= PAGE_RESERVED)
#define page_set_available(page) (page->flags |= PAGE_AVAILABLE)
#define page_available(page) (page->flags & PAGE_AVAILABLE)

#define round_up_page(x)	round_up((x), PAGE_SIZE)
#define round_down_page(x)	round_down((x), PAGE_SIZE)

void page_init(void);
void set_pde(unsigned long* pde, unsigned long pa, uint32_t flag);
void set_pte(unsigned long* pte, unsigned long pa, uint32_t flag);
void page_map(unsigned long *pgdir, unsigned long va, unsigned long pa, size_t size, uint32_t flag);
void page_table_dump(unsigned long *pgdir, unsigned long va, size_t size);
void enable_paging(unsigned long cr3);

#define virt_to_phys(x) ((uintptr_t)(x) - KERNEL_VADDR_SHIFT)
#define phys_to_virt(x) ((uintptr_t)(x) + KERNEL_VADDR_SHIFT)
#define phys_to_pfn(x) ((x) >> PAGE_SHIFT)

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

void init_free_area(unsigned long start, unsigned long end);
struct page *alloc_pages(unsigned int n);
void free_pages(struct page *page);

#define alloc_page() alloc_pages(1)

void kernel_map(unsigned long kva, unsigned long pa, size_t size, uint32_t flag);
void kernel_page_table_dump(unsigned long va, size_t size);
