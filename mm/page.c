#include <asm-generic/cpu.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <x86.h>

/* #define PAGE_DEBUG */

/* use buddy algorithm to allocate free pages,
 * support physical address up to 4GB, totally 1024 * 1024 pages.
 */

#define TOTAL_PAGES (1024 * 1024)
#define MAX_ORDER 10

struct page pages[TOTAL_PAGES];
static struct list_node free_lists[MAX_ORDER + 1];

#ifdef PAGE_DEBUG
#define dump_page(page) \
	do { \
		pr_debug("page:", hex(page), " index:", dec(page_to_pfn(page)), \
			 " order:", dec(page->order), " free:", dec(page->free)); \
	} while (0);
#else
#define dump_page(page)
#endif


unsigned long page_to_pfn(struct page *page)
{
	return page - pages;
}

struct page *pfn_to_page(unsigned long pfn)
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
	dump_page(page);
}

void init_free_area(unsigned long start, unsigned long end)
{
	unsigned long split_start, split_end, addr;

	start = round_up_page(start);
	end = round_down_page(end);

	pr_debug("free range:", range(start, end));

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
			dump_page(page);

			assert(i == page->order);

			while (page->order > order)
			{
				page->order--;
				buddy = page_buddy(page);
				assert(page_available(buddy));
				page_set_free(buddy, buddy->order);
			}

			page->free = false;
			dump_page(page);
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
static unsigned long* get_pte(unsigned long *pgdir, unsigned long va)
{
	unsigned long* pde = pgdir + pde_index(va);
	struct page *page;
	unsigned long page_pa;
	unsigned long *pt;

	if (!(*pde & PTE_P)) {
		page = alloc_page();
		dump_page(page);
		page_pa = page_to_phys(page);
		memset((void *)phys_to_virt(page_pa), 0, PAGE_SIZE);

		set_pde(pde, page_pa, PDE_P | PDE_W);
	}

	pt = (void *)phys_to_virt(page_base(*pde));
	return pt + pte_index(va);
}

void set_pde(unsigned long* pde, unsigned long pa, uint32_t flag)
{
	*pde = pa | flag | PDE_P;
}

void set_pte(unsigned long* pte, unsigned long pa, uint32_t flag)
{
	*pte = pa | flag | PTE_P;
}

void page_map(unsigned long *pgdir, unsigned long va, unsigned long pa, size_t size, uint32_t flag)
{
	unsigned long *pte;
	unsigned long end;

	va = round_down_page(va);
	pa = round_down_page(pa);
	end = round_up_page(va + size);

	pr_info("map: <", hex(va), "->", hex(pa), "> size:", hex(size));

	while (va < end) {
		pte = get_pte(pgdir, va);
		set_pte(pte, pa, PTE_P | flag);

		va += PAGE_SIZE;
		pa += PAGE_SIZE;
	}
}

void page_table_dump(unsigned long *pgdir, unsigned long va, size_t size)
{
	unsigned long pde, pte, va_base, pte_va;
	unsigned long *pt;
	int i, j;

	for (i = 0; i < 1 << 10; i++) {
		pde = pgdir[i];
		if (!(pde & PDE_P))
			continue;

		va_base = i * 1024 * PAGE_SIZE;
		if (va_base < va || va_base > (va + size))
			continue;

		pt = (void *)phys_to_virt(page_base(pde));
		printk("|-[", dec(i), "] va_base:", hex(va_base), " -> pde:", hex(pde), "\n");

		for (j = 0; j < 1 << 10; j++) {
			pte = pt[j];
			if (!(pte & PTE_P))
				continue;

			pte_va = va_base + j * PAGE_SIZE;
			printk("\t|-[", dec(j), "] va:", hex(pte_va), " -> pte:", hex(pte), "\n");
		}
	}
}

void enable_paging(unsigned long cr3)
{
	unsigned long cr0;

	lcr3(cr3);
	cr0 = rcr0();
	cr0 |= CR0_PE | CR0_PG | CR0_AM | CR0_WP | CR0_NE | CR0_TS | CR0_EM | CR0_MP;
	cr0 &= ~(CR0_TS | CR0_EM);
	lcr0(cr0);
}

void page_init(void)
{
	int order;

	for (order = 0; order <= MAX_ORDER; order++) {
		free_lists[order].prev = &free_lists[order];
		free_lists[order].next = &free_lists[order];
	}
}
