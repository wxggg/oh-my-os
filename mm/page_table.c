#include <asm-generic/cpu.h>
#include <x86.h>
#include <stdio.h>
#include <memory.h>
#include <vmalloc.h>
#include <register.h>
#include <debug.h>
#include <atomic.h>

#define MODULE "page table"
#define MODULE_DEBUG 1

static inline void tlb_invalidate(unsigned long *pgdir, unsigned long va)
{
	if (rcr3() == virt_to_phys(pgdir))
		invlpg((void *)va);
}

/*
 * get_pte - if pte not exist, allocate a new page
 */
static unsigned long *get_pte(unsigned long *pgdir, unsigned long va)
{
	unsigned long *pde = pgdir + pde_index(va);
	struct page *page;
	unsigned long page_pa;
	unsigned long *pt;

	if (!(*pde & PTE_P)) {
		page = alloc_page(GFP_NORMAL);
		page_pa = page_to_phys(page);
		memset((void *)phys_to_virt(page_pa), 0, PAGE_SIZE);

		set_pde(pde, page_pa, PDE_P | PDE_W);
	}

	pt = (void *)phys_to_virt(page_base(*pde));
	return pt + pte_index(va);
}

void set_pde(unsigned long *pde, unsigned long pa, uint32_t flag)
{
	*pde = pa | flag | PDE_P;
}

void page_map(unsigned long *pgdir, unsigned long va, unsigned long pa,
	      size_t size, uint32_t flag)
{
	unsigned long *pte;
	unsigned long offset;

	va = round_down_page(va);
	pa = round_down_page(pa);

	pr_debug("map: <", hex(va), "->", hex(pa), "> size:", hex(size));

	for (offset = 0; offset < size; offset += PAGE_SIZE) {
		pte = get_pte(pgdir, va);
		*pte = pa | flag | PTE_P;
		tlb_invalidate(pgdir, va);

		va += PAGE_SIZE;
		pa += PAGE_SIZE;
	}
}

void page_unmap(unsigned long *pgdir, unsigned long va, size_t size)
{
	unsigned long *pte;
	unsigned long offset;

	va = round_down_page(va);

	pr_debug("unmap: va:", hex(va), " size:", hex(size));

	for (offset = 0; offset < size; offset += PAGE_SIZE) {
		pte = get_pte(pgdir, va);
		*pte = 0;
		tlb_invalidate(pgdir, va);

		va += PAGE_SIZE;
	}
}

void page_table_dump(unsigned long *pgdir, unsigned long va, size_t size)
{
	unsigned long pde, pte, va_base, pte_va;
	unsigned long *pt;
	int i, j;

	pr_info("dump page table for va:", hex(va), " size:", hex(size));

	for (i = 0; i < 1 << 10; i++) {
		pde = pgdir[i];
		if (!(pde & PDE_P))
			continue;

		va_base = i * 1024 * PAGE_SIZE;
		if (va_base < va)
			continue;

		if (va_base >= (va + size))
			return;

		pt = (void *)phys_to_virt(page_base(pde));
		printk("|-[", dec(i), "] va_base:", hex(va_base),
		       " -> pde:", hex(pde), "\n");

		for (j = 0; j < 1 << 10; j++) {
			pte = pt[j];
			if (!(pte & PTE_P))
				continue;

			pte_va = va_base + j * PAGE_SIZE;
			printk("\t|-[", dec(j), "] va:", hex(pte_va),
			       " -> pte:", hex(pte), "\n");

			if (pte_va > (va + size))
				return;
		}
	}
}

void enable_paging(unsigned long cr3)
{
	unsigned long cr0;

	lcr3(cr3);
	cr0 = rcr0();
	cr0 |= CR0_PE | CR0_PG | CR0_AM | CR0_WP | CR0_NE | CR0_TS | CR0_EM |
	       CR0_MP;
	cr0 &= ~(CR0_TS | CR0_EM);
	lcr0(cr0);
}

void *page_address(struct page *page)
{
	if (!test_bit(PAGE_HIGHMEM, &page->flags))
		return (void *)phys_to_virt(page_to_phys(page));

	return vmap(&page, 1);
}
