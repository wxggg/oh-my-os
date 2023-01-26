#include <asm-generic/mmu.h>
#include <types.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <x86.h>
#include <string.h>

#define E820_MAX	20
#define E820_MEM	1
#define E820_RSV	2

struct e820_map {
	uint64_t addr;
	uint64_t size;
	uint32_t type;
};

/*
 * struct e820map
 * physical memory topo info
 */
struct e820 {
	int n;
	struct e820_map map[E820_MAX];
};

struct kernel_memory {
	struct e820 *e820;

	struct range kernel_range;

	pa_t pa_max;
	uint32_t page_max;
	struct page *pages; /* va */

	struct page *pgdir_page;
	pa_t cr3;
	pa_t* pgdir;
};

struct kernel_memory kmemory = {};

#define VPT 0xfac00000

const char *range_str(struct range *r)
{
	char *buf = get_format_buffer();
	size_t size = get_format_size();
	size_t len;
	char *p = buf;

	p[0] = '<';
	p[1] = '0';
	p[2] = 'x';
	p += 3;
	size -= 3;

	to_hex(r->start, p, size);
	len = strlen(p);
	p += len;
	size -= len;

	p[0] = ',';
	p[1] = ' ';
	p += 2;
	size -= 2;

	to_hex(r->end, p, size);
	len = strlen(p);
	p += len;
	p[0] = '>';

	return buf;
}

static void scan_memory_slot(void)
{
	struct e820 *e820 = (struct e820 *) __kva(0x8000);
	struct e820_map *map;
	extern char end[];
	struct range free_range;
	size_t i, index;

	kmemory.e820 = e820;
	kmemory.pa_max = 0x0;

	for (i = 0; i < e820->n; i++) {
		map = &e820->map[i];

		pr_info("scan memory slot: <", xstr(map->addr), ", ",
			xstr(map->addr + map->size - 1), "> type:", dstr(map->type));

		if (map->type == E820_MEM) {
			if (map->addr < (size_t)__pa(end)
					&& (size_t)__pa(end) < (map->addr + map->size)) {
				index = i;
			}

			kmemory.pa_max = max(kmemory.pa_max, map->addr + map->size);
		}
	}


	kmemory.page_max = kmemory.pa_max / PAGE_SIZE;
	kmemory.pages = (struct page *) round_up((pa_t)end, PAGE_SIZE);

	kmemory.kernel_range.start = 0x0;
	kmemory.kernel_range.end = __pa((kva_t)kmemory.pages
			+ kmemory.page_max * sizeof(struct page));
	kmemory.kernel_range.end = round_up(kmemory.kernel_range.end, PAGE_SIZE) - 1;

	map = &e820->map[index];
	free_range.start = kmemory.kernel_range.end + 1;
	free_range.end = map->addr + map->size - 1;

	pr_info("kernel range:", range_str(&kmemory.kernel_range),
		" free range:", range_str(&free_range));

	manage_free_memory(free_range.start, free_range.end);

	/* memory after kernel should be added to memory manger */
	for (size_t i = index + 1; i < e820->n; i++) {
		map = &e820->map[i];

		if (map->type == E820_MEM) {
			manage_free_memory(map->addr, map->addr + map->size);
		}
	}
}

void memory_init(void)
{
	scan_memory_slot();

	page_init(kmemory.pages);

	kmemory.pgdir_page = alloc_page();
	kmemory.cr3 = page_address(kmemory.pgdir_page);
	kmemory.pgdir = (pa_t *)__kva(kmemory.pgdir);

	/*
	 * insert one item in pgdir to map virtual page
	 * table to VPT. One pde covers 1<<10 * 1<<12 = 4MB
	 */
	set_pde(kmemory.pgdir + pde_index(VPT), kmemory.cr3, PTE_P | PTE_W);

	/* map kernel memory to the start of 0xC0000000 */
	page_map(kmemory.pgdir, KERNEL_VADDR_SHIFT, 0x0,
		 range_size(&kmemory.kernel_range), PTE_W);
}
