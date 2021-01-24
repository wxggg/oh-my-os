#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct page_manager {
	struct page *pages;

	struct page *free_pages;
	size_t total;

	size_t allocated;
	size_t index;
};

struct page_manager manager = {};

pa_t page_address(struct page *page)
{
	return (pa_t)((page - manager.pages) << PAGE_OFFSET);
}

struct page * find_page(pa_t pa)
{
	return &manager.pages[pa >> PAGE_OFFSET];
}

void manage_free_memory(pa_t start, pa_t end)
{
	println(4, "manage free memory: start:0x", xstr(start),
			   "\tend:0x", xstr(end));

	start = round_up(start, PAGE_SIZE);
	end = round_down(end, PAGE_SIZE);

	manager.total = (end - start) / PAGE_SIZE;
	manager.free_pages = find_page(start);
	manager.index = 0;
	manager.allocated = 0;

	for (size_t i = 0; i < manager.total; i++) {
		manager.free_pages[i].used = false;
	}
}

struct page * alloc_page()
{
	struct page *page = NULL;

	if (manager.allocated >= manager.total)
		return NULL;

	while (manager.free_pages[manager.index % manager.total].used == true) {
		manager.index++;
	}

	manager.index = manager.index % manager.total;
	page = &manager.free_pages[manager.index];
	page->used = true;
	manager.allocated++;

	return page;
}

void free_page(struct page *page)
{
	page->used = false;
	manager.allocated--;
}

/*
 * get_pte - if pte not exist, allocate a new page
 */
static pa_t* get_pte(pa_t *pgdir, va_t va)
{
	pa_t* pde = pgdir + pde_index(va);
	struct page *page;
	pa_t page_pa;

	if (!(*pde & PTE_P)) {

		page = alloc_page();
		page_pa = page_address(page);
		memset((void *)__kva(page_pa), 0, PAGE_SIZE);

		set_pde(pde, page_pa, PTE_P | PTE_W);
	}

	return (pa_t *)(*pde) + pte_index(va);
}

void set_pde(pa_t* pde, pa_t pa, uint32_t perm)
{
	*pde = pa | perm | PTE_P;
}

void set_pte(pa_t* pte, pa_t pa, uint32_t perm)
{
	*pte = pa | perm | PTE_P;
}

void page_map(pa_t *pgdir, va_t va, pa_t pa, size_t size, uint32_t perm)
{
	size_t n;
	pa_t *pte;


	n = page_number(page_round_up(size + page_offset(va)));
	va = page_round_down(va);
	pa = page_round_down(pa);

	println(8, "page_map va:", xstr(va),
			" to pa:", xstr(pa),
			" size:", xstr(size),
			" n:", istr(n));

	for(; n > 0; n--) {
		pte = get_pte(pgdir, va);
		set_pte(pte, pa, PTE_P | perm);

		va += PAGE_SIZE;
		pa += PAGE_SIZE;

		/* println(4, "\tva:", xstr(va), "\tpa:", xstr(pa)); */
	}
}

void page_init(struct page *pages)
{
	manager.pages = pages;
}
