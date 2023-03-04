#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>

/* use buddy algorithm to allocate free pages,
 * support physical address up to 4GB, totally 1024 * 1024 pages.
 */

#define TOTAL_PAGES (1024 * 1024)
#define MAX_ORDER 10

struct page pages[TOTAL_PAGES];
struct list_node free_lists[MAX_ORDER + 1];

unsigned long page_to_pfn(struct page *page)
{
	return page - pages;
}

struct page *pfn_to_page(unsigned long long pfn)
{
	return &pages[pfn];
}

static inline struct page *page_buddy(struct page *page)
{
	return &pages[page_to_pfn(page) ^ (1 << page->order)];
}

static void page_set_free(struct page *page, unsigned int order)
{
	page->free = true;
	page->order = order;
	list_add_to_head(&free_lists[order], &page->node);
}

void init_free_area(unsigned long long start, unsigned long long end)
{
	unsigned long long split_start, split_end, addr;

	start = round_up_page(start);
	end = round_down_page(end);

	if (start >= end)
		return;

	/* split <start, end> to three parts */
	split_start = round_up(start, PAGE_SIZE << MAX_ORDER);
	split_end = round_down(end, PAGE_SIZE << MAX_ORDER);

	if (split_start >= split_end) {
		/* init as single page */
		addr = start;
		while (addr < end) {
			page_set_free(phys_to_page(addr), 0);
			addr += PAGE_SIZE;
		}
	} else {

		/* init as single page */
		addr = start;
		while (addr < split_start) {
			page_set_free(phys_to_page(addr), 0);
			addr += PAGE_SIZE;
		}

		/* init as max order pages */
		while (addr < split_end) {
			page_set_free(phys_to_page(addr), MAX_ORDER);
			addr += PAGE_SIZE << MAX_ORDER;
		}

		/* init as single page */
		while (addr < end) {
			page_set_free(phys_to_page(addr), 0);
			addr += PAGE_SIZE;
		}
	}

	for (addr = start; addr < end; addr += PAGE_SIZE)
		page_set_available(phys_to_page(addr));
}

static inline unsigned int get_order(unsigned int n)
{
	unsigned long order = 0;
	while ((1 << order) <= n && order <= MAX_ORDER)
		order++;

	return order - 1;
}

struct page *alloc_pages(unsigned int n)
{
	unsigned long order;
	unsigned long i;
	struct list_node *node;
	struct page *page, *buddy;

	if (n > 1 << MAX_ORDER)
		return NULL;

	order = get_order(n);

	for (i = order; i <= MAX_ORDER; i++)
	{
		if (!list_empty(&free_lists[i]))
		{
			node = list_next(&free_lists[i]);
			list_remove(node);
			page = container_of(node, struct page, node);

			assert(i == page->order);

			while (page->order > order)
			{
				page->order--;
				buddy = page_buddy(page);
				assert(page_available(buddy));
				page_set_free(buddy, buddy->order);
			}

			page->free = false;
			return page;
		}
	}

	return NULL;
}

void free_pages(struct page *page)
{
	struct page *buddy;

	while (page->order < MAX_ORDER)
	{
		buddy = page_buddy(page);

		if (!page_available(buddy) || !page->free)
			break;

		if (buddy < page)
			page = buddy;

		page->order++;
	}

	page_set_free(page, page->order);
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
		page_pa = page_to_phys(page);
		memset((void *)phys_to_virt(page_pa), 0, PAGE_SIZE);

		set_pde(pde, page_pa, PTE_P | PTE_W);
	}

	return (pa_t *)(*pde) + pde_index(va);
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
	pa_t *pte;
	va_t end;

	va = round_down_page(va);
	pa = round_down_page(pa);
	end = round_up_page(va + size);

	pr_info("map: <", xstr(va), "->", xstr(pa), "> size:", xstr(size));

	while (va < end) {
		pte = get_pte(pgdir, va);
		set_pte(pte, pa, PTE_P | perm);
		va += PAGE_SIZE;
		pa += PAGE_SIZE;
	}
}
