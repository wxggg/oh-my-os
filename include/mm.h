#pragma once

#include <list.h>

struct vm_area {
	unsigned long start;
	unsigned long end;
	bool free;

	struct page **pages;
	unsigned long nr_pages;

	struct list_node node;
	struct list_node free_node;
};

struct mm_context {
	unsigned long *pgdir;
};

