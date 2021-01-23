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
void page_init(struct page *pages, size_t n);
