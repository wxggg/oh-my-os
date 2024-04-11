#include <fs.h>
#include <schedule.h>
#include <stdio.h>
#include <kmalloc.h>
#include <debug.h>
#include <fifo.h>

#define MODULE "thread"
#define MODULE_DEBUG 1

static int test_thread(void *arg)
{
	while (1) {
		schedule();
	};

	return 0;
}

static int thread_test(struct file *file, vector *vec)
{
	thread_run(test_thread, NULL, -1);
	return 0;
}

static struct file_operations thread_fops = {
	.exec = thread_test,
};

static int test_consumer(void *arg)
{
	struct fifo *fifo = arg;
	int v;

	while (1) {
		if (!fifo_out(fifo, &v, sizeof(v)))
			printk(dec(v), "\n");

		if (v == 255)
			break;
	}

	printk("consumer exit\n");
	return 0;
}

static int test_producer(void *arg)
{
	struct fifo *fifo = arg;
	int i;

	for (i = 0; i < 256; i++) {
		while (fifo_in(fifo, &i, sizeof(i)) == -ENOSPC) {
		}
	}

	printk("producer exit\n");
	return 0;
}

static int fifo_test(struct file *file, vector *vec)
{
	struct fifo *fifo;

	fifo = kmalloc(sizeof(*fifo));
	if (!fifo)
		return -ENOMEM;

	fifo->data = kmalloc(64);
	if (!fifo->data) {
		kfree(fifo);
		return -ENOMEM;
	}

	fifo_init(fifo, fifo->data, 64);

	thread_run(test_producer, fifo, 1);
	thread_run(test_consumer, fifo, 2);

	kfree(fifo->data);
	kfree(fifo);
	return 0;
}

static struct file_operations fifo_fops = {
	.exec = fifo_test,
};

int usr_thread_init(void)
{
	int ret;
	struct file *file;

	ret = binfs_create_file("thread_test", &thread_fops, NULL, &file);
	if (ret)
		return ret;

	ret = binfs_create_file("fifo_test", &fifo_fops, NULL, &file);
	if (ret)
		return ret;

	return 0;
}
