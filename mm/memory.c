#include <asm-generic/mmu.h>
#include <types.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <x86.h>
#include <string.h>
#include <kmalloc.h>

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

	unsigned long start;
	unsigned long end;

	struct page *pgdir_page;
	pa_t cr3;
	pa_t* pgdir;
};

struct kernel_memory kmemory = {};

#define VPT 0xfac00000

static void scan_memory_slot(void)
{
	struct e820 *e820 = (struct e820 *) phys_to_virt(0x8000);
	struct e820_map *map;
	extern char end[];
	unsigned long free_start, free_end;
	int i, index;

	kmemory.e820 = e820;
	for (i = 0; i < e820->n; i++) {
		map = &e820->map[i];

		pr_info("scan memory slot: ",
			"type: ", dec(map->type),
			"range: ", range(map->addr, map->addr + map->size - 1), "\t\t");

		if (map->type == E820_MEM) {
			if (map->addr < virt_to_phys(end) &&
					virt_to_phys(end) < (map->addr + map->size)) {
				index = i;
			}
		}
	}

	kmemory.start = 0x0;
	kmemory.end = round_up_page((uintptr_t)end);

	map = &e820->map[index];
	free_start = kmemory.end + 1;
	free_end = map->addr + map->size - 1;

	pr_info("kernel range: ", range(kmemory.start, kmemory.end),
		"free range: ", range(free_start, free_end));

	init_free_area(free_start, free_end);

	/* memory after kernel should be added to memory manger */
	for (i = index + 1; i < e820->n; i++) {
		map = &e820->map[i];

		if (map->type == E820_MEM) {
			init_free_area(map->addr, map->addr + map->size);
		}
	}
}

void memory_init(void)
{
	scan_memory_slot();

	kmemory.pgdir_page = alloc_page();
	kmemory.cr3 = page_to_phys(kmemory.pgdir_page);
	kmemory.pgdir = (void *)phys_to_virt(kmemory.pgdir);

	/*
	 * insert one item in pgdir to map virtual page
	 * table to VPT. One pde covers 1<<10 * 1<<12 = 4MB
	 */
	set_pde(kmemory.pgdir + pde_index(VPT), kmemory.cr3, PTE_P | PTE_W);

	/* map kernel memory to the start of 0xC0000000 */
	page_map(kmemory.pgdir, KERNEL_VADDR_SHIFT, 0x0,
		 kmemory.end - kmemory.start + 1, PTE_W);

	pr_info("memory init success");
}
