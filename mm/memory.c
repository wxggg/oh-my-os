#include <asm-generic/mmu.h>
#include <asm-generic/cpu.h>
#include <types.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <x86.h>
#include <string.h>
#include <kmalloc.h>
#include <vmalloc.h>
#include <schedule.h>
#include <assert.h>

#define MODULE "memory"
#define MODULE_DEBUG 0

#define E820_MAX 20
#define E820_RAM 1
#define E820_RSV 2

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
static struct pseudodesc gdt_desc = { sizeof(gdt) - 1, (uintptr_t)gdt };

#define IO_BASE 0xf0000000
#define VPT 0xfac00000

unsigned long kernel_start_pfn, kernel_end_pfn;
unsigned long linear_start_pfn, linear_end_pfn;
unsigned long highmem_start_pfn, highmem_end_pfn;

static const char *e820_type_str(unsigned int type)
{
	switch (type) {
	case E820_RAM:
		return "System RAM";
	case E820_RSV:
		return "Reserved";
	default:
		return "Unknown type";
	}
}

static void scan_memory_slot(void)
{
	struct e820 *e820 = (struct e820 *)phys_to_virt(0x8000);
	struct e820_map *map;
	extern char end[];
	unsigned long free_end_pfn;
	int i, index;

	pr_info("BIOS-provided physical RAM map:");

	for (i = 0; i < e820->n; i++) {
		map = &e820->map[i];

		pr_info("BIOS-e820: [", e820_type_str(map->type), " \t",
			range(map->addr, map->addr + map->size - 1), "]\t\t");

		if (map->type == E820_RAM) {
			if (map->addr < virt_to_phys(end) &&
			    virt_to_phys(end) < (map->addr + map->size)) {
				index = i;
			}
		}
	}

	kernel_start_pfn = 0;
	kernel_end_pfn = phys_to_pfn(round_up_page(virt_to_phys(end)));

	map = &e820->map[index];

	free_end_pfn = phys_to_pfn(map->addr + map->size);

	linear_start_pfn = kernel_end_pfn;

	highmem_start_pfn = phys_to_pfn(PHYS_HIGHMEM_START);

	if (free_end_pfn <= highmem_start_pfn) {
		highmem_end_pfn = highmem_start_pfn;
		linear_end_pfn = free_end_pfn;
	} else {
		highmem_end_pfn = free_end_pfn;
		linear_end_pfn = highmem_start_pfn;
	}

	pr_info("physical memory frame number ranges:");
	pr_info("kernel:\t", range(kernel_start_pfn, kernel_end_pfn));
	pr_info("linear:\t", range(kernel_end_pfn, highmem_start_pfn));
	pr_info("highmem:\t", range(highmem_start_pfn, highmem_end_pfn));

	add_free_pages(kernel_end_pfn, free_end_pfn);

	/* memory after kernel should be added to memory manger */
	for (i = index + 1; i < e820->n; i++) {
		map = &e820->map[i];
		if (map->type == E820_RAM)
			add_free_pages(phys_to_pfn(map->addr),
				       phys_to_pfn(map->addr + map->size));
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
	seg->avl = 0;
	seg->l = 0;
	seg->db = 1; /* 32-bit segment */
	seg->gran = gran;
	seg->base_31_24 = (base >> 24) & 0xff;
}

static void gdt_init(void)
{
	seg_init(&gdt[SEG_KTEXT], STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_KERNEL,
		 1);
	seg_init(&gdt[SEG_KDATA], STA_W, 0x0, 0xFFFFFFFF, DPL_KERNEL, 1);
	seg_init(&gdt[SEG_UTEXT], STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_USER, 1);
	seg_init(&gdt[SEG_UDATA], STA_W, 0x0, 0xFFFFFFFF, DPL_USER, 1);

	asm volatile("lgdt (%0)" ::"r"(&gdt_desc));
	asm volatile("movw %%ax, %%gs" ::"a"(USER_DS));
	asm volatile("movw %%ax, %%fs" ::"a"(USER_DS));
	asm volatile("movw %%ax, %%es" ::"a"(KERNEL_DS));
	asm volatile("movw %%ax, %%ds" ::"a"(KERNEL_DS));
	asm volatile("movw %%ax, %%ss" ::"a"(KERNEL_DS));
	/* reload cs */
	asm volatile("ljmp %0, $1f\n 1:\n" ::"i"(KERNEL_CS));
}

void start_paging(uint32_t *pgdir)
{
	/* since the gdt has not been set to identity map,
	 * so the linear got after gdt is:
	 * linear address = virtual address - KERNEL_VIRT_BASE
	 * so the fetched instruction will minus KERNEL_VIRT_BASE
	 * however the map of linear address [0, ~] to physical address [0, ~]
	 * has not been set up yet, so we need to set up the map temporarily to
	 * avoid page fault when execute instruction after enable paging.
	 */
	pgdir[0] = pgdir[pde_index(KERNEL_VIRT_BASE)];

	/* enable paging */
	enable_paging(virt_to_phys(pgdir));

	/* reconfigure the gdt to identity map virtual address to linear address. */
	gdt_init();

	/* the map of linear address [0, ~] is unnessary now, since the gdt
	 * has changed the map of virtual address to linear address to identity map.
	 */
	pgdir[0] = 0;
}

void kernel_map(unsigned long va, unsigned long pa, size_t size, uint32_t flag)
{
	return page_map(current->proc->mm->pgdir, va, pa, size, flag);
}

void kernel_unmap(unsigned long va, size_t size)
{
	return page_unmap(current->proc->mm->pgdir, va, size);
}

void kernel_page_table_dump(unsigned long va, size_t size)
{
	page_table_dump(current->proc->mm->pgdir, va, size);
}

void memory_init(void)
{
	struct mm_context *mm;
	struct page *page;
	unsigned long cr3;

	page_init();

	scan_memory_slot();

	mm = kmalloc(sizeof(*current->proc->mm));
	assert(mm);

	current->proc->mm = mm;

	page = alloc_page(GFP_NORMAL);
	cr3 = page_to_phys(page);
	mm->pgdir = (void *)phys_to_virt(cr3);

	/*
	 * insert one item in pgdir to map virtual page
	 * table to VPT. One pde covers 1<<10 * 1<<12 = 4MB
	 */
	set_pde(mm->pgdir + pde_index(VPT), cr3, PDE_P | PDE_W);

	/* map linear area */
	kernel_map(KERNEL_VIRT_BASE, 0x0, linear_end_pfn << PAGE_SHIFT, PTE_W);

	start_paging(mm->pgdir);

	vmalloc_init();

	kmalloc_init();

	pr_info("memory init success");
}
