#include <asm-generic/cpu.h>
#include <x86.h>
#include <stdio.h>
#include <memory.h>

static inline void tlb_invalidate(unsigned long *pgdir, unsigned long va)
{
	if (rcr3() == virt_to_phys(pgdir))
		invlpg((void *)va);
}

/*
 * get_pte - if pte not exist, allocate a new page
 */
static unsigned long* get_pte(unsigned long *pgdir, unsigned long va)
{
	unsigned long* pde = pgdir + pde_index(va);
	struct page *page;
	unsigned long page_pa;
	unsigned long *pt;

	if (!(*pde & PTE_P)) {
		page = alloc_page(GFP_HIGHMEM);
		page_pa = page_to_phys(page);
		memset((void *)phys_to_virt(page_pa), 0, PAGE_SIZE);

		set_pde(pde, page_pa, PDE_P | PDE_W);
	}

	pt = (void *)phys_to_virt(page_base(*pde));
	return pt + pte_index(va);
}

void set_pde(unsigned long* pde, unsigned long pa, uint32_t flag)
{
	*pde = pa | flag | PDE_P;
}

void page_map(unsigned long *pgdir, unsigned long va, unsigned long pa, size_t size, uint32_t flag)
{
	unsigned long *pte;
	unsigned long end;

	va = round_down_page(va);
	pa = round_down_page(pa);
	end = round_up_page(va + size);

	pr_debug("map: <", hex(va), "->", hex(pa), "> size:", hex(size));

	while (va < end) {
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
	unsigned long end;

	va = round_down_page(va);
	end = round_up_page(va + size);

	pr_debug("unmap: ", range(va, end));

	while (va < end) {
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

	for (i = 0; i < 1 << 10; i++) {
		pde = pgdir[i];
		if (!(pde & PDE_P))
			continue;

		va_base = i * 1024 * PAGE_SIZE;
		if (va_base < va || va_base > (va + size))
			continue;

		pt = (void *)phys_to_virt(page_base(pde));
		printk("|-[", dec(i), "] va_base:", hex(va_base), " -> pde:", hex(pde), "\n");

		for (j = 0; j < 1 << 10; j++) {
			pte = pt[j];
			if (!(pte & PTE_P))
				continue;

			pte_va = va_base + j * PAGE_SIZE;
			printk("\t|-[", dec(j), "] va:", hex(pte_va), " -> pte:", hex(pte), "\n");
		}
	}
}

void enable_paging(unsigned long cr3)
{
	unsigned long cr0;

	lcr3(cr3);
	cr0 = rcr0();
	cr0 |= CR0_PE | CR0_PG | CR0_AM | CR0_WP | CR0_NE | CR0_TS | CR0_EM | CR0_MP;
	cr0 &= ~(CR0_TS | CR0_EM);
	lcr0(cr0);
}

