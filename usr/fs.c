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

	if (vector_size(vec) > 2) {
		printk("cd: invalid arguments ", dec(vector_size(vec)), "\n");
		return -EINVAL;
	}

	if (vector_size(vec) == 1) {
		current_dir = root;
		return 0;
	}

	name = vector_at(vec, string *, 1);

	if (!strcmp(name->str, "..")) {
		if (current_dir->parent)
			current_dir = current_dir->parent;
		return 0;
	}

	if (!strcmp(name->str, "."))
		return 0;

	if (!strcmp(name->str, "~")) {
		current_dir = root;
		return 0;
	}

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

static int do_cat(vector *vec)
{
	int ret;
	struct file *file;
	string *name, *content;

	if (vector_size(vec) != 2) {
		printk("cat: invalid arguments ", dec(vector_size(vec)), "\n");
		return -EINVAL;
	}

	name = vector_at(vec, string *, 1);

	file = dir_find_file(current_dir, name->str);
	if (!file) {
		printk("cat: no such file ", name->str, "\n");
		return -ENOENT;
	}

	if (!file->fops->read) {
		printk("cat: read is not supported for ", file->name, "\n");
		return -EINVAL;
	}

	content = ksalloc();

	ret = file->fops->read(content);
	if (ret)
		goto err_free_content;

	printk(content->str, "\n");

	ksfree(content);
	return 0;

err_free_content:
	ksfree(content);
	return ret;
}

static struct file_operations cat_fops = {
	.exec = do_cat,
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

	ret = binfs_create_file("cat", &cat_fops, &file);
	if (ret)
		return ret;

	return 0;
}
