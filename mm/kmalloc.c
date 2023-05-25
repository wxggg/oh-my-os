#include <types.h>
#include <memory.h>

#define EARLY_SLAB_MAX 1024

struct early_slab {
	int slab_size;
	char *buf;
	void *free_stack[EARLY_SLAB_MAX];
	uint32_t free_count;
};

#define DECLARE_EARYLY_SLAB(x) \
	static char g_slab_buf_##x[EARLY_SLAB_MAX * x]; \
	static struct early_slab g_early_slab_##x = {	\
		.buf = g_slab_buf_##x,			\
		.slab_size = x,				\
	};

#define EARLY_SLAB(x) ((struct early_slab *)&g_early_slab_##x)

DECLARE_EARYLY_SLAB(8);
DECLARE_EARYLY_SLAB(16);
DECLARE_EARYLY_SLAB(32);
DECLARE_EARYLY_SLAB(64);
DECLARE_EARYLY_SLAB(128);
DECLARE_EARYLY_SLAB(256);

uint32_t get_slab_size(uint32_t size)
{
	uint32_t slab_size = 8;

	if (slab_size > 256)
		return PAGE_SIZE;

	while (slab_size < size)
		slab_size <<= 1;

	return slab_size;
}

static struct early_slab *get_early_slab(uint32_t slab_size)
{
	switch (slab_size) {
		case 8:
			return EARLY_SLAB(8);
		case 16:
			return EARLY_SLAB(16);
		case 32:
			return EARLY_SLAB(32);
		case 64:
			return EARLY_SLAB(64);
		case 128:
			return EARLY_SLAB(128);
		case 256:
			return EARLY_SLAB(256);
		default:
			return NULL;
	}

	return NULL;
}

void kmalloc_early_init(void)
{
	struct early_slab *slab;
	uint32_t slab_size;
	uint32_t i;

	for (slab_size = 8; slab_size <= 256; slab_size <<= 1) {
		slab = get_early_slab(slab_size);

		for (i = 0; i < EARLY_SLAB_MAX; i++) {
			slab->free_stack[i] = (void *)(slab->buf + slab_size * i);
		}

		slab->free_count = EARLY_SLAB_MAX;
	}
}

void *kmalloc(size_t size)
{
	struct early_slab *slab;
	uint32_t slab_size = get_slab_size(size);

	if (slab_size == 0 || slab_size >= PAGE_SIZE)
		return NULL;

	slab = get_early_slab(slab_size);
	if (!slab)
		return NULL;

	if (slab->free_count == 0)
		return NULL;

	return slab->free_stack[--slab->free_count];
}

void kfree(void *ptr)
{
	struct early_slab *slab;
	uint32_t slab_size;

	for (slab_size = 8; slab_size <= 256; slab_size <<= 1) {
		slab = get_early_slab(slab_size);
		if (in_range(ptr, slab->buf, slab_size * 1024)) {
			slab->free_stack[slab->free_count++] = ptr;
			return;
		}
	}
}

