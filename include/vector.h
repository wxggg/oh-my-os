#pragma once

#include <types.h>

typedef struct vector {
	void *base;
	size_t element_size;
	size_t size;
	size_t capacity;
} vector;

vector *__vector_create(size_t element_size);
void __vector_destroy(vector *vec);
void __vector_at(vector *vec, unsigned int index, void *ele);
int __vector_push(vector *vec, void *ele);
int __vector_pop(vector *vec, void *ele);

static inline size_t vector_size(vector *vec)
{
	return vec->size;
}

#define vector_create(type) __vector_create(sizeof(type))

#define vector_push(vec, type, value)     \
	do {                              \
		type __a = (type)value;   \
		__vector_push(vec, &__a); \
	} while (0);

#define vector_pop(vec, type)            \
	({                               \
		type __a;                \
		__vector_pop(vec, &__a); \
		__a;                     \
	})


static inline bool vector_empty(vector *vec)
{
	return vec->size == 0;
}

#define vector_at(vec, type, index) ((type *)vec->base)[index]
