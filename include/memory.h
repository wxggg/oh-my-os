#include <asm-generic/mmu.h>
#include <types.h>

typedef uintptr_t pa_t;
typedef uintptr_t kva_t;
typedef uintptr_t va_t;

struct page {
	uint32_t flags;
	bool used;
};

void memory_init(void);

void manage_free_memory(pa_t start, pa_t end);
struct page * alloc_page();
void free_page(struct page *page);
void page_init(struct page *pages);

pa_t page_address(struct page *page);
struct page * find_page(pa_t pa);

#define pde_index(x) (((x) >> 22) & 0x3ff)
#define pte_index(x) (((x) >> 12) & 0x3ff)

#define page_number(x) ((x) >> 12)
#define page_base(x) ((x) & ~0xfff)
#define page_offset(x) ((x) & 0xfff)

#define page_round_up(x)	round_up((x), PAGE_SIZE)
#define page_round_down(x)	round_down((x), PAGE_SIZE)

void set_pde(pa_t* pde, pa_t pa, uint32_t perm);
void set_pte(pa_t* pte, pa_t pa, uint32_t perm);
void page_map(pa_t *pgdir, va_t va, pa_t pa, size_t size, uint32_t perm);
