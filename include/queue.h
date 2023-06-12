#pragma once

#include <types.h>

typedef struct queue {
	void *base;
	size_t element_size;
	unsigned int front;
	unsigned int rear;
	size_t capacity;
} queue;

queue *__queue_create(size_t element_size);
void __queue_front(queue *que, void *ele);
void __queue_rear(queue *rear, void *ele);
int __enqueue(queue* que, void *ele);
int __dequeue(queue *que, void *ele);

#define queue_create(type)		\
	__queue_create(sizeof(type))

#define enqueue(que, type, value)	\
	do {				\
		type __a = (type)value;	\
		__enqueue(que, &__a);\
	} while (0);

#define dequeue(que, type)		\
	({				\
		type __a;		\
		__dequeue(que, &__a);	\
		__a;			\
	 })

#define queue_front(que, type)		\
	((type *)que->base)[que->front]

#define queue_rear(que, type)		\
	((type *)que->base)[(que->rear + que->capacity - 1) % que->capacity]

#define queue_empty(que)			\
	(que->front == que->rear)

#define queue_full(que)			\
	(((que->rear + 1) % que->capacity) == que->front)

#define queue_size(que)			\
	((que->rear + que->capacity - que->front) % que->capacity)
