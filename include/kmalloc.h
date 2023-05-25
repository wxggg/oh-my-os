#include <types.h>

void kmalloc_early_init(void);

void *kmalloc(size_t size);

void kfree(void *p);

uint32_t get_slab_size(uint32_t size);
