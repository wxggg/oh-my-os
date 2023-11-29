#include <asm-generic/cpu.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <x86.h>
#include <assert.h>
#include <bitops.h>
#include <fs.h>

/* #define PAGE_DEBUG */

/* use buddy algorithm to allocate free pages,
 * support physical address up to 4GB, totally 1024 * 1024 pages.
 */

#define TOTAL_PAGES (1024 * 1024)
#define MAX_ORDER 10

struct page pages[TOTAL_PAGES];

static struct list_node highmem_free_lists[MAX_ORDER + 1];
static struct list_node free_lists[MAX_ORDER + 1];

#define dump_page(page) \
	do { \
		pr_debug("page:", hex(page), " pfn:", hex(page_to_pfn(page)), \
			 " order:", dec(page->order), " flags:", hex(page->flags), \
			 " ", is_bit_set(page->flags, PAGE_HIGHMEM) ? "highmem" : "linear"); \
	} while (0);

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
	set_bit(page->flags, PAGE_FREE);
	page->order = order;

	if (is_bit_set(page->flags, PAGE_HIGHMEM))
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

	assert(count == nr_pages, "invalid nr_pages:", dec(nr_pages),
		       ", order:", dec(order));
}

void add_free_pages(unsigned long start_pfn, unsigned long end_pfn)
{
	struct page *page;
	unsigned long split_start, split_end, pfn;
	bool highmem;

	assert(start_pfn <= end_pfn, " invalid pfn range ",
		       range(start_pfn, end_pfn));

	if (start_pfn >= highmem_start_pfn) {
		highmem = true;
	} else if (end_pfn <= highmem_start_pfn) {
		highmem = false;
	} else {
		add_free_pages(start_pfn, highmem_start_pfn);
		add_free_pages(highmem_start_pfn, end_pfn);
		return;
	}

	pr_info("add free pages, pfn:", range(start_pfn, end_pfn), ", ",
		highmem ? "high mem" : "linear mem");

	for (pfn = start_pfn; pfn < end_pfn; pfn++) {
		page = pfn_to_page(pfn);
		set_bit(page->flags, PAGE_VALID);
		if (highmem)
			set_bit(page->flags, PAGE_HIGHMEM);
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
		list = get_free_list(gfp_mask, i);
		if (!list_empty(list))
		{
			node = list_next(list);
			list_remove(node);
			page = container_of(node, struct page, node);

			assert(i == page->order, "invalid order ", pair(i, page->order));

			while (page->order > order)
			{
				page->order--;
				buddy = page_buddy(page);
				assert(is_bit_set(buddy->flags, PAGE_VALID),
				       "buddy-", dec(page_to_pfn(buddy)), " is not available");
				free_page(buddy, page->order);
			}

			clear_bit(page->flags, PAGE_FREE);
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

		if (!is_bit_set(buddy->flags, PAGE_VALID) ||
		    !is_bit_set(buddy->flags, PAGE_FREE))
			break;

		list_remove(&buddy->node);
		if (buddy < page)
			page = buddy;

		page->order++;
	}

	free_page(page, page->order);
}

void page_init(void)
{
	int order;

	for (order = 0; order <= MAX_ORDER; order++) {
		list_init(&highmem_free_lists[order]);
		list_init(&free_lists[order]);
	}
}

static int dump_free_list(string *s)
{
	unsigned int i;

	for (i = 0; i <= MAX_ORDER; i++) {
		ksappend_kv(s, "order:", i);
		ksappend_kv(s, " highmem-", i);
		ksappend_kv(s, " ", list_size(&highmem_free_lists[i]));
		ksappend_kv(s, "\t\tlinear-", i);
		ksappend_kv(s, " ", list_size(&free_lists[i]));
		ksappend_str(s, "\n");
	}

	return 0;
}

static struct file_operations dump_page_fops = {
	.read = dump_free_list,
};

int page_init_late(void)
{
	struct file *file;

	create_file("free_pages", &dump_page_fops, sys, &file);

	return 0;
}
