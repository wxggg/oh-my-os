#pragma once

#include <memory.h>

int vmalloc_init(void);

void *__vmalloc(unsigned long size, gfp_t gfp_mask);
void *vmalloc(unsigned long size);
void vfree(void *addr);
void *vmap(struct page **pages, unsigned int nr_pages);
void vunmap(void *addr);

static inline bool is_vmalloc_addr(unsigned long addr)
{
	return addr >= VMALLOC_START && addr < VMALLOC_END;
}

int vmalloc_init_late(void);
