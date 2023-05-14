#include <asm-generic/mmu.h>
#include <types.h>
#include <list.h>

typedef uintptr_t pa_t;
typedef uintptr_t kva_t;
typedef uintptr_t va_t;

struct page {
	unsigned long flags;
	unsigned long order;
	struct list_node node;
	bool free;
};

struct range {
	uintptr_t start;
	uintptr_t end;
};

const char *range_str(struct range *r);
static inline size_t range_size(struct range *r)
{
	return (r->end - r->start + 1);
}

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

#define page_set_reserved(page) (page->flags &= PAGE_RESERVED)
#define page_set_available(page) (page->flags &= PAGE_AVAILABLE)
#define page_available(page) (page->flags & PAGE_AVAILABLE)

#define round_up_page(x)	round_up((x), PAGE_SIZE)
#define round_down_page(x)	round_down((x), PAGE_SIZE)

void set_pde(pa_t* pde, pa_t pa, uint32_t perm);
void set_pte(pa_t* pte, pa_t pa, uint32_t perm);
void page_map(pa_t *pgdir, va_t va, pa_t pa, size_t size, uint32_t perm);

#define virt_to_phys(x) ((uintptr_t)x - KERNEL_VADDR_SHIFT)
#define phys_to_virt(x) ((uintptr_t)x + KERNEL_VADDR_SHIFT)
#define phys_to_pfn(x) ((x) >> PAGE_OFFSET)

unsigned long page_to_pfn(struct page *page);
struct page *pfn_to_page(unsigned long long pfn);

static inline unsigned long long page_to_phys(struct page *page)
{
	return PAGE_SIZE << page_to_pfn(page);
}

static inline struct page *phys_to_page(unsigned long long phys)
{
	return pfn_to_page(phys_to_pfn(phys));
}

void init_free_area(unsigned long long start, unsigned long long end);
struct page *alloc_pages(unsigned int n);
void free_pages(struct page *page);

#define alloc_page() alloc_pages(1)
