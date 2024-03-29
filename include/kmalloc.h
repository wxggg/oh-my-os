#include <types.h>

void kmalloc_early_init(void);
void kmalloc_init(void);

void *kmalloc(size_t size);
void kfree(void *p);

int kmalloc_init_late(void);
