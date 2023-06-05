#pragma once

#include <memory.h>

int vmalloc_init(void);

void *__vmalloc(unsigned long size, gfp_t gfp_mask);
void *vmalloc(unsigned long size);
void vfree(void *addr);
