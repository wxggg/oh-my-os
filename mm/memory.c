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

	struct range kernel_range;

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
	struct e820 *e820 = (struct e820 *) phys_to_virt(0x8000);
	struct e820_map *map;
	extern char end[];
	struct range free_range;
	size_t i, index;

	kmemory.e820 = e820;
	for (i = 0; i < e820->n; i++) {
		map = &e820->map[i];

		pr_info("scan memory slot: <", xstr(map->addr), ", ",
			xstr(map->addr + map->size - 1), "> type:", dstr(map->type));

		if (map->type == E820_MEM) {
			if (map->addr < virt_to_phys(end) &&
			    virt_to_phys(end) < (map->addr + map->size)) {
				index = i;
			}
		}
	}

	kmemory.kernel_range.start = 0x0;
	kmemory.kernel_range.end = round_up_page((uintptr_t)end);

	map = &e820->map[index];
	free_range.start = kmemory.kernel_range.end + 1;
	free_range.end = map->addr + map->size - 1;

	pr_info("kernel range:", range_str(&kmemory.kernel_range),
		" free range:", range_str(&free_range));

	init_free_area(free_range.start, free_range.end);

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
		 range_size(&kmemory.kernel_range), PTE_W);

	kmalloc_init();
}
