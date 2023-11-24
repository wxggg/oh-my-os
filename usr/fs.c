#include <fs.h>
#include <kernel.h>
#include <stdio.h>
#include <assert.h>

struct directory *current_dir;

static int do_ls(vector *vec)
{
	struct directory *d;
	struct file *f;
	struct list_node *node, *head;

	head = &current_dir->dir_list;
	for (node = head->next; node != head; node = node->next) {
		d = container_of(node, struct directory, node);
		printk(d->name, " ");
	}

	head = &current_dir->file_list;
	for (node = head->next; node != head; node = node->next) {
		f = container_of(node, struct file, node);
		printk(f->name, " ");
	}

	printk("\n");

	return 0;
}

static struct file_operations ls_fops = {
	.exec = do_ls,
};

static int do_cd(vector *vec)
{
	struct directory *dir;
	string *name;

	if (vector_size(vec) != 2) {
		printk("cd: invalid arguments ", dec(vector_size(vec)));
		return -EINVAL;
	}

	name = vector_at(vec, string *, 1);

	dir = dir_find_dir(current_dir, name->str);
	if (!dir) {
		printk("cd: no such directory ", name->str, "\n");
		return -ENOENT;
	}

	current_dir = dir;
	return 0;
}

static struct file_operations cd_fops = {
	.exec = do_cd,
};

int usr_fs_init(void)
{
	int ret;
	struct file *file;

	current_dir = root;

	ret = binfs_create_file("ls", &ls_fops, &file);
	if (ret)
		return ret;

	ret = binfs_create_file("cd", &cd_fops, &file);
	if (ret)
		return ret;

	return 0;
}
