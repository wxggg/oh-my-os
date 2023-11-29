#ifndef __FS_H__
#define __FS_H__

#include <list.h>
#include <string.h>
#include <vector.h>

struct file_operations {
	int (*read)(string *s);
	void (*write)(string *s);
	int (*exec)(vector *vec);
};

struct file {
	const char *name;

	struct directory *parent;
	struct list_node node;
	struct file_operations *fops;
};

struct directory {
	const char *name;
	struct directory *parent;

	struct list_node node;
	struct list_node file_list;
	struct list_node dir_list;
};

extern struct directory *root;
extern struct directory *bin;
extern struct directory *proc;
extern struct directory *sys;
extern struct directory *current_dir;

int fs_init(void);

int create_file(const char *name, struct file_operations *fops,
		struct directory *parent, struct file **file);
int remove_file(struct file *file);

int create_directory(const char *name, struct directory *parent,
		     struct directory **dir);
int remove_directory(struct directory *dir);

struct file *dir_find_file(struct directory *dir, const char *name);
struct directory *dir_find_dir(struct directory *dir, const char *name);

struct file *binfs_find_file(const char *name);
int binfs_create_file(const char *name, struct file_operations *fops,
		      struct file **file);
int binfs_remove_file(const char *name);

int procfs_create_dir(const char *name, struct directory **dir);
int procfs_remove_dir(const char *name);

#endif /* __FS_H__ */
