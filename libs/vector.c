#include <assert.h>
#include <string.h>
#include <kmalloc.h>
#include <vector.h>

#define MODULE "vector"
#define MODULE_DEBUG 0

#define MIN_CAPACITY 8

vector *__vector_create(size_t element_size)
{
	vector *vec;

	vec = kmalloc(sizeof(*vec));
	if (!vec)
		return NULL;

	vec->base = kmalloc(MIN_CAPACITY * element_size);
	if (!vec->base) {
		kfree(vec);
		return NULL;
	}

	vec->element_size = element_size;
	vec->size = 0;
	vec->capacity = MIN_CAPACITY;
	return vec;
}

void __vector_destroy(vector *vec)
{
	kfree(vec->base);
	kfree(vec);
}

static inline void *__ele(vector *vec, unsigned index)
{
	return vec->base + vec->element_size * index;
}

void __vector_at(vector *vec, unsigned int index, void *ele)
{
	assert(index < vec->size);
	memcpy(ele, __ele(vec, index), vec->element_size);
}

static int __vector_expand(vector *vec)
{
	void *new_base;

	new_base = kmalloc(vec->element_size * vec->capacity * 2);
	if (!new_base)
		return -ENOMEM;

	memcpy(new_base, vec->base, vec->element_size * vec->size);
	kfree(vec->base);
	vec->base = new_base;
	vec->capacity <<= 1;
	return 0;
}

int __vector_push(vector *vec, void *ele)
{
	if (vec->size == vec->capacity && __vector_expand(vec))
		return -ENOMEM;

	memcpy(__ele(vec, vec->size), ele, vec->element_size);
	vec->size++;
	return 0;
}

int __vector_pop(vector *vec, void *ele)
{
	if (vec->size == 0)
		return -ENOENT;

	memcpy(ele, __ele(vec, vec->size - 1), vec->element_size);
	vec->size--;
	return 0;
}
