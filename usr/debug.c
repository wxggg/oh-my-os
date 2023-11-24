#include <debug.h>
#include <fs.h>
#include <usr.h>

static int dump_stack_fops_exec(vector *vec)
{
	dump_stack();
	return 0;
}

static struct file_operations debug_fops = {
	.exec = dump_stack_fops_exec,
};

int usr_debug_init(void)
{
	int ret;
	struct file *file;

	ret = binfs_create_file("dump_stack", &debug_fops, &file);
	if (ret)
		return ret;

	return 0;
}
