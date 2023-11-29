#include <debug.h>
#include <fs.h>
#include <usr.h>

static int do_bt(vector *vec)
{
	dump_stack();
	return 0;
}

static struct file_operations debug_fops = {
	.exec = do_bt,
};

int usr_debug_init(void)
{
	int ret;
	struct file *file;

	ret = binfs_create_file("bt", &debug_fops, &file);
	if (ret)
		return ret;

	return 0;
}
