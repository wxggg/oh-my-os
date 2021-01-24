#include <asm-generic/mmu.h>
#include <types.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <x86.h>

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
	size_t index;

	/* [start end) */
	pa_t start;
	pa_t end;
	size_t size;

	pa_t pa_max;
	uint32_t page_max;
	struct page *pages; /* va */

	pa_t free_start;
	pa_t free_end;

	struct page *pgdir_page;
	pa_t cr3;
	pa_t* pgdir;
};

struct kernel_memory kmemory = {};

#define VPT 0xfac00000

static void scan_memory_slot(void)
{
	struct e820 *e820 = (struct e820 *) __kva(0x8000);
	struct e820_map *map, *kmap;
	extern char end[];

	kmemory.e820 = e820;
	kmemory.pa_max = 0x0;

	for (size_t i = 0; i < e820->n; i++) {
		map = &e820->map[i];
		println(8,
				"\ti:",		 istr(i),
				"\taddr:0x", xstr(map->addr),
				"\tsize:0x", xstr(map->size),
				"\ttype:0x", xstr(map->type));

		if (map->type == E820_MEM) {
			if (map->addr < (size_t)__pa(end)
					&& (size_t)__pa(end) < (map->addr + map->size)) {
				kmemory.index = i;
				kmap = map;
			}

			kmemory.pa_max = max(kmemory.pa_max, map->addr + map->size);
		}
	}

	kmemory.start = 0x0;

	kmemory.page_max = kmemory.pa_max / PAGE_SIZE;
	kmemory.pages = (struct page *) round_up((pa_t)end, PAGE_SIZE);

	kmemory.end = __pa((kva_t)kmemory.pages
			+ kmemory.page_max * sizeof(struct page));
	kmemory.size = kmemory.end - kmemory.start;

	kmemory.free_start = kmemory.end;
	kmemory.free_end = kmap->addr + kmap->size;

	printk("kernel memory:");
	println(2, "\tindex:", xstr(kmemory.index));
	println(2, "\tstart:0x", xstr(kmemory.start));
	println(2, "\tend:0x", xstr(kmemory.end));
	println(2, "\tsize:0x", xstr(kmemory.size));
	println(2, "\tpage_max:", istr(kmemory.page_max));
	println(2, "\tfree_start:0x", xstr(kmemory.free_start));
	println(2, "\tfree_end:0x", xstr(kmemory.free_end));

	manage_free_memory(kmemory.free_start, kmemory.free_end);

	/* memory after kernel should be added to memory manger */
	for (size_t i = kmemory.index + 1; i < e820->n; i++) {
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
	page_map(kmemory.pgdir, KERNEL_VADDR_SHIFT, 0x0, kmemory.size, PTE_W);
}
