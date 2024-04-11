#pragma once

#include <types.h>

struct fifo {
	unsigned int in;
	unsigned int out;
	void *data;
	unsigned int size;
};

#define OFFSET_TO_PTR(base, offset) ((void *)((unsigned int)base + offset))

void fifo_init(struct fifo *fifo, void *data, unsigned int size);
int fifo_in(struct fifo *fifo, void *data, unsigned int size);
int fifo_out(struct fifo *fifo, void *data, unsigned int size);
int fifo_out_peek(struct fifo *fifo, void *data, unsigned int size);
int fifo_skip(struct fifo *fifo, unsigned int size);

