#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

struct page_manager {
	struct page *pages;

	struct page *free_pages;
	size_t total;

	size_t allocated;
	size_t index;
};

struct page_manager manager = {};

static struct page * find_page(pa_t pa)
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

void page_init(struct page *pages, size_t n)
{
	manager.pages = pages;
	manager.total = n;
	manager.index = 0;
	manager.allocated = 0;

	for (size_t i = 0; i < n; i++) {
		pages[i].used = false;
	}
}
