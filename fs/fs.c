#include <kmalloc.h>
#include <fs.h>
#include <kernel.h>
#include <stdio.h>

struct directory *root;
struct directory *bin;
struct directory *proc;
struct directory *sys;

struct file *dir_find_file(struct directory *dir, const char *name)
{
	struct file *file;
	struct list_node *node, *head = &dir->file_list;

	for (node = head->next; node != head; node = node->next) {
		file = container_of(node, struct file, node);
		if (!strcmp(name, file->name))
			return file;
	}

	return NULL;
}

struct directory *dir_find_dir(struct directory *dir, const char *name)
{
	struct directory *d;
	struct list_node *node, *head = &dir->dir_list;

	for (node = head->next; node != head; node = node->next) {
		d = container_of(node, struct directory, node);
		if (!strcmp(name, d->name))
			return d;
	}

	return NULL;
}

int create_file(const char *name, struct file_operations *fops,
		struct directory *parent, struct file **file)
{
	struct file *f;

	f = kmalloc(sizeof(*f));
	if (!f)
		return -ENOMEM;

	f->parent = parent;
	f->name = name;
	f->fops = fops;

	list_insert(&parent->file_list, &f->node);

	*file = f;
	return 0;
}

int remove_file(struct file *file)
{
	list_remove(&file->node);
	kfree(file);
	return 0;
}

int create_directory(const char *name, struct directory *parent,
		     struct directory **dir)
{
	struct directory *d;

	d = kmalloc(sizeof(*d));
	if (!d)
		return -ENOMEM;

	d->name = name;
	d->parent = parent;

	list_init(&d->file_list);
	list_init(&d->dir_list);

	if (parent)
		list_insert(&parent->dir_list, &d->node);

	*dir = d;
	return 0;
}

int remove_directory(struct directory *dir)
{
	struct file *file;
	struct directory *d;
	struct list_node *node, *head;

	/* remove all sub file */
	head = &dir->file_list;
	for (node = head->next; node != head; node = node->next) {
		file = container_of(node, struct file, node);
		remove_file(file);
	}

	/* remove all sub directory */
	head = &dir->dir_list;
	for (node = head->next; node != head; node = node->next) {
		d = container_of(node, struct directory, node);
		remove_directory(d);
	}

	list_remove(&dir->node);
	kfree(dir);
	return 0;
}

struct file *binfs_find_file(const char *name)
{
	return dir_find_file(bin, name);
}

int binfs_create_file(const char *name, struct file_operations *fops,
		      struct file **file)
{
	return create_file(name, fops, bin, file);
}

int binfs_remove_file(const char *name)
{
	struct file *file;

	file = dir_find_file(bin, name);
	if (!file)
		return -ENOENT;

	return remove_file(file);
}

int procfs_create_dir(const char *name, struct directory **dir)
{
	return create_directory(name, proc, dir);
}

int procfs_remove_dir(const char *name)
{
	struct directory *dir;

	dir = dir_find_dir(proc, name);
	if (!dir)
		return -ENOENT;

	return remove_directory(dir);
}

int fs_init(void)
{
	int ret;

	ret = create_directory("/", NULL, &root);
	if (ret)
		return ret;

	ret = create_directory("bin", root, &bin);
	if (ret)
		return ret;

	ret = create_directory("proc", root, &proc);
	if (ret)
		return ret;

	ret = create_directory("sys", root, &sys);
	if (ret)
		return ret;

	return 0;
}
