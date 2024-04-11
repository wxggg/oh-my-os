#include <fs.h>
#include <schedule.h>
#include <stdio.h>
#include <kmalloc.h>
#include <debug.h>

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

int usr_thread_init(void)
{
	int ret;
	struct file *file;

	ret = binfs_create_file("thread_test", &thread_fops, NULL, &file);
	if (ret)
		return ret;

	return 0;
}
