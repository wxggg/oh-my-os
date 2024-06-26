#pragma once
#include <list.h>
#include <lock.h>

struct kmem_cache {
	struct list_node slabs_full;
	struct list_node slabs_partial;
	unsigned int size;
	spinlock_t lock;
};

void kmem_cache_free(struct kmem_cache *cache, void *obj);
void *kmem_cache_alloc(struct kmem_cache *cache);
int kmem_cache_create(struct kmem_cache *cache, size_t size);
