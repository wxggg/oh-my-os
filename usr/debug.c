#include <debug.h>
#include <fs.h>
#include <usr.h>

static int do_bt(struct file *file, vector *vec)
{
	dump_stack();
	return 0;
}

static struct file_operations debug_fops = {
	.exec = do_bt,
};

static int do_dmesg(struct file *file, vector *vec)
{
	dmesg();
	return 0;
}

static struct file_operations dmesg_fops = {
	.exec = do_dmesg,
};

int usr_debug_init(void)
{
	int ret;
	struct file *file;

	ret = binfs_create_file("bt", &debug_fops, NULL, &file);
	if (ret)
		return ret;

	ret = binfs_create_file("dmesg", &dmesg_fops, NULL, &file);
	if (ret)
		return ret;

	return 0;
}
