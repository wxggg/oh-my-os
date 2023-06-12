#include <assert.h>
#include <string.h>
#include <kmalloc.h>
#include <queue.h>

#define MIN_CAPACITY 8

queue *__queue_create(size_t element_size)
{
	queue *que;

	que = kmalloc(sizeof(*que));
	if (!que)
		return NULL;

	que->base = kmalloc(MIN_CAPACITY * element_size);
	if (!que->base) {
		kfree(que);
		return NULL;
	}

	que->element_size = element_size;
	que->front = 0;
	que->rear = 0;
	que->capacity = MIN_CAPACITY;
	return que;
}

static inline void *__ele(queue *que, unsigned int index)
{
	return que->base + que->element_size * index;
}

void __queue_front(queue *que, void *ele)
{
	assert(que->front < que->capacity);
	memcpy(ele, __ele(que, que->front), que->element_size);
}

void __queue_rear(queue *que, void *ele)
{
	assert(que->rear < que->capacity);
	memcpy(ele, __ele(que, (que->rear + que->capacity - 1) % que->capacity),
		que->element_size);
}

int __queue_expand(queue *que)
{
	void *new_base;

	new_base = kmalloc(que->element_size * que->capacity * 2);
	if (!new_base)
		return -ENOMEM;

	if (que->front < que->rear) {
		/* 0    front -> rear    capacity */
		memcpy(new_base, __ele(que, que->front),
		       queue_size(que) * que->element_size);
	} else {
		/* 0 -> rear    front -> capacity */
		memcpy(new_base, __ele(que, que->front),
		       (que->capacity - que->front) * que->element_size);

		memcpy(new_base + (que->capacity - que->front) * que->element_size,
		       que->base, que->rear * que->element_size);
	}

	que->rear = queue_size(que);
	que->front = 0;
	que->base = new_base;
	que->capacity <<= 1;
	return 0;
}

int __enqueue(queue *que, void *ele)
{
	int ret;

	if (queue_full(que)) {
		ret = __queue_expand(que);
		if (ret)
			return ret;
	}

	memcpy(__ele(que, que->rear), ele, que->element_size);
	que->rear = (que->rear + 1) % que->capacity;
	return 0;
}

int __dequeue(queue *que, void *ele)
{
	if (queue_empty(que))
		return -ENOENT;

	memcpy(ele, __ele(que, que->front), que->element_size);
	que->front = (que->front + 1) % que->capacity;
	return 0;
}
