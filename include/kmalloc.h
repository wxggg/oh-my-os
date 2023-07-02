#include <types.h>

void kmalloc_early_init(void);
void kmalloc_init(void);

void *kmalloc(size_t size);
void kfree(void *p);

void kmalloc_dump(void);
