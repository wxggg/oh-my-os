#include <asm-generic/cpu.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <x86.h>
#include <assert.h>
#include <bitops.h>

/* #define PAGE_DEBUG */

/* use buddy algorithm to allocate free pages,
 * support physical address up to 4GB, totally 1024 * 1024 pages.
 */

#define TOTAL_PAGES (1024 * 1024)
#define MAX_ORDER 10

struct page pages[TOTAL_PAGES];

static struct list_node highmem_free_lists[MAX_ORDER + 1];
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

static inline void free_page(struct page *page, unsigned int order)
{
	page->free = true;
	page->order = order;

	if (page_is_highmem(page))
		list_insert(&highmem_free_lists[order], &page->node);
	else
		list_insert(&free_lists[order], &page->node);
}

static inline void init_free_pages(struct page *start_page, unsigned long nr_pages,
				   unsigned int order)
{
	unsigned long count;

	for (count = 0; count < nr_pages; count += 1 << order)
		free_page(start_page + count, order);

	assert_notrace(count == nr_pages, "invalid nr_pages:", dec(nr_pages),
		       ", order:", dec(order));
}

void add_free_pages(unsigned long start_pfn, unsigned long end_pfn)
{
	struct page *page;
	unsigned long split_start, split_end, pfn;
	bool highmem;

	assert_notrace(start_pfn <= end_pfn, " invalid pfn range ",
		       range(start_pfn, end_pfn));

	if (end_pfn <= highmem_end_pfn) {
		highmem = true;
	} else if (start_pfn >= highmem_end_pfn) {
		highmem = false;
	} else {
		add_free_pages(start_pfn, highmem_end_pfn);
		add_free_pages(highmem_end_pfn, end_pfn);
		return;
	}

	pr_info("add free pages, pfn:", range(start_pfn, end_pfn), ", ",
		highmem ? "high mem" : "vmalloc mem");

	for (pfn = start_pfn; pfn < end_pfn; pfn++) {
		page = pfn_to_page(pfn);
		page_set_valid(page);
		if (highmem)
			page_set_highmem(page);
	}

	/* split <start, end> to three parts */
	split_start = round_up(start_pfn, 1 << MAX_ORDER);
	split_end = round_down(end_pfn, 1 << MAX_ORDER);

	pr_debug("start end:", range(start_pfn, end_pfn),
		 ", split start end:", range(split_start, split_end));

	if (split_start >= split_end) {
		/* init as single page */
		init_free_pages(pfn_to_page(start_pfn), (end_pfn - start_pfn), 0);
	} else {

		/* init as single page */
		init_free_pages(pfn_to_page(start_pfn), (split_start - start_pfn), 0);

		/* init as max order pages */
		init_free_pages(pfn_to_page(split_start), (split_end - split_start), MAX_ORDER);

		/* init as single page */
		init_free_pages(pfn_to_page(split_end), (end_pfn - split_end), 0);
	}
}

static struct list_node *get_free_list(gfp_t gfp_mask, unsigned int order)
{
	if (gfp_mask & GFP_HIGHMEM)
		return &highmem_free_lists[order];
	else
		return &free_lists[order];
}

struct page *alloc_pages(gfp_t gfp_mask, unsigned int order)
{
	unsigned long i;
	struct list_node *node, *list;
	struct page *page, *buddy;

	if (order > MAX_ORDER)
		return NULL;

	for (i = order; i <= MAX_ORDER; i++)
	{
		list = get_free_list(gfp_mask, order);
		if (!list_empty(list))
		{
			node = list_next(list);
			list_remove(node);
			page = container_of(node, struct page, node);
			dump_page(page);

			assert(i == page->order);

			while (page->order > order)
			{
				page->order--;
				buddy = page_buddy(page);
				assert(page_is_valid(buddy), "buddy-", dec(page_to_pfn(buddy)), " is not available");
				free_page(buddy, buddy->order);
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

		if (!page_is_valid(buddy) || !page->free)
			break;

		if (buddy < page)
			page = buddy;

		page->order++;
	}

	free_page(page, page->order);
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
		page = alloc_page(GFP_HIGHMEM);
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
		list_init(&highmem_free_lists[order]);
		list_init(&free_lists[order]);
	}
}
