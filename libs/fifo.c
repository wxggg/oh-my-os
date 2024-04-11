#include <stdlib.h>
#include <string.h>
#include <fifo.h>
#include <x86.h>

void fifo_init(struct fifo *fifo, void *data, unsigned int size)
{
	fifo->in = 0;
	fifo->out = 0;

	fifo->data = data;
	fifo->size = size;
}

static inline unsigned int fifo_len(struct fifo *fifo)
{
	return (fifo->in + fifo->size - fifo->out) % fifo->size;
}

static inline unsigned int fifo_avail(struct fifo *fifo)
{
	return (fifo->size - 1 - fifo_len(fifo));
}

static inline bool fifo_is_empty(struct fifo *fifo)
{
	return fifo->in == fifo->out;
}

static inline bool fifo_is_full(struct fifo *fifo)
{
	return (fifo->in + 1) % fifo->size == fifo->out;
}

static void fifo_copy_in(struct fifo *fifo, const void *src, unsigned int len,
			 unsigned int off)
{
	unsigned int size = fifo->size;
	unsigned int l;

	l = min(len, size - off);

	memcpy(fifo->data + off, src, l);
	memcpy(fifo->data, src + l, len - l);

	smp_wmb();
}

int fifo_in(struct fifo *fifo, void *data, unsigned int size)
{
	if (fifo_avail(fifo) < size)
		return -ENOSPC;

	fifo_copy_in(fifo, data, size, fifo->in);

	fifo->in = (fifo->in + size) % fifo->size;

	return 0;
}

static void fifo_copy_out(struct fifo *fifo, void *dst, unsigned int len,
			  unsigned int off)
{
	unsigned int size = fifo->size;
	unsigned int l;

	l = min(len, size - off);

	memcpy(dst, fifo->data + off, l);
	memcpy(dst + l, fifo->data, len - l);

	smp_wmb();
}

int fifo_out(struct fifo *fifo, void *data, unsigned int size)
{
	if (fifo_len(fifo) < size)
		return -ENOENT;

	fifo_copy_out(fifo, data, size, fifo->out);

	fifo->out = (fifo->out + size) % fifo->size;

	return 0;
}

int fifo_out_peek(struct fifo *fifo, void *data, unsigned int size)
{
	if (fifo_len(fifo) < size)
		return -ENOENT;

	fifo_copy_out(fifo, data, size, fifo->out);

	return 0;
}

int fifo_skip(struct fifo *fifo, unsigned int size)
{
	if (fifo_len(fifo) < size)
		return -ENOENT;

	fifo->out = (fifo->out + size) % fifo->size;
	return 0;
}
