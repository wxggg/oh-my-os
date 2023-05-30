#include <asm-generic/mmu.h>
#include <asm-generic/cpu.h>
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
	unsigned long cr3;
	unsigned long* pgdir;
};

struct kernel_memory kmemory = {};

struct seg_desc {
        unsigned lim_15_0 : 16;
        unsigned base_15_0 : 16;
        unsigned base_23_16 : 8;
        unsigned type : 4;
        unsigned s : 1;
        unsigned dpl : 2;
        unsigned present : 1;
        unsigned lim_19_16 : 4;
        unsigned avl : 1;
        unsigned l : 1;
        unsigned db : 1;
        unsigned gran : 1;
        unsigned base_31_24 : 8;
};

static struct seg_desc gdt[SEG_MAX] = { 0 };
static struct pseudodesc gdt_desc = {
	sizeof(gdt) - 1, (uintptr_t)gdt
};

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
			"type: ", dec(map->type), ", ",
			"range: ", range(map->addr, map->addr + map->size - 1), "\t\t");

		if (map->type == E820_MEM) {
			if (map->addr < virt_to_phys(end) &&
					virt_to_phys(end) < (map->addr + map->size)) {
				index = i;
			}
		}
	}

	kmemory.start = 0x0;
	kmemory.end = round_up_page((uintptr_t)(virt_to_phys(end))) - 1;

	map = &e820->map[index];
	free_start = kmemory.end + 1;
	free_end = map->addr + map->size - 1;

	pr_info("kernel range: ", range(kmemory.start, kmemory.end), ", ",
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

static void seg_init(struct seg_desc *seg, u8 type, u32 base, u32 limit,
		     u32 dpl, bool gran)
{
	if (gran)
		limit = limit >> 12;

	seg->lim_15_0 = limit & 0xffff;
	seg->base_15_0 = base & 0xffff;
	seg->base_23_16 = (base >> 16) & 0xff;
	seg->type = type;
	seg->s = 1; /* code or data segment */
	seg->dpl = dpl;
	seg->present = 1;
	seg->lim_19_16 = (limit >> 16) & 0xf;
	seg->avl =  0;
	seg->l = 0;
	seg->db = 1; /* 32-bit segment */
	seg->gran = gran;
	seg->base_31_24 = (base >> 24) & 0xff;
}

static void gdt_init(void)
{
	seg_init(&gdt[SEG_KTEXT], STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_KERNEL, 1);
	seg_init(&gdt[SEG_KDATA], STA_W,         0x0, 0xFFFFFFFF, DPL_KERNEL, 1);
	seg_init(&gdt[SEG_UTEXT], STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_USER, 1);
	seg_init(&gdt[SEG_UDATA], STA_W,         0x0, 0xFFFFFFFF, DPL_USER, 1);

	asm volatile ("lgdt (%0)" :: "r" (&gdt_desc));
	asm volatile ("movw %%ax, %%gs" :: "a" (USER_DS));
	asm volatile ("movw %%ax, %%fs" :: "a" (USER_DS));
	asm volatile ("movw %%ax, %%es" :: "a" (KERNEL_DS));
	asm volatile ("movw %%ax, %%ds" :: "a" (KERNEL_DS));
	asm volatile ("movw %%ax, %%ss" :: "a" (KERNEL_DS));
	/* reload cs */
	asm volatile ("ljmp %0, $1f\n 1:\n" :: "i" (KERNEL_CS));
}

void kernel_map(unsigned long kva, unsigned long pa, size_t size, uint32_t flag)
{
	return page_map(kmemory.pgdir, kva, pa, size, flag);
}

void kernel_page_table_dump(unsigned long va, size_t size)
{
	page_table_dump(kmemory.pgdir, va, size);
}

void memory_init(void)
{
	page_init();

	scan_memory_slot();

	kmemory.pgdir_page = alloc_page();
	kmemory.cr3 = page_to_phys(kmemory.pgdir_page);
	kmemory.pgdir = (void *)phys_to_virt(kmemory.cr3);

	pr_info("kmemory.pgdir:", hex(kmemory.pgdir), ", "
		"cr3:", hex(kmemory.cr3), ", ",
		"pgdir_page:", hex(kmemory.pgdir_page));

	/*
	 * insert one item in pgdir to map virtual page
	 * table to VPT. One pde covers 1<<10 * 1<<12 = 4MB
	 */
	set_pde(kmemory.pgdir + pde_index(VPT), kmemory.cr3, PDE_P | PDE_W);

	/* map kernel memory to the start of 0xC0000000 */
	page_map(kmemory.pgdir, KERNEL_VADDR_SHIFT, 0x0, 0x38000000, PTE_W);

	kmemory.pgdir[0] = kmemory.pgdir[pde_index(KERNEL_VADDR_SHIFT)];

	enable_paging(kmemory.cr3);

        gdt_init();

	kmemory.pgdir[0] = 0;

	pr_info("memory init success");
}
