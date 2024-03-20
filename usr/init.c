#include <string.h>
#include <fs.h>
#include <stdio.h>
#include <schedule.h>
#include <usr.h>

struct thread *shell;

static int run_shell(void *arg)
{
	struct file *file;
	vector *vec = vector_create(string *);
	string *s = ksalloc();
	string *sub, *cmd;

	while (1) {
		readline(s);

		if (!s->str || !s->length)
			continue;

		kssplit(s, ' ', vec);

		cmd = vector_at(vec, string *, 0);

		file = binfs_find_file(cmd->str);
		if (file && file->fops->exec) {
			file->fops->exec(file, vec);
		} else {
			printk("command not found: ", cmd->str, "\n");
		}

		while (!vector_empty(vec)) {
			sub = vector_pop(vec, string *);
			ksfree(sub);
		}
	}

	ksfree(s);
	__vector_destroy(vec);
	return 0;
}

int start_new_shell(void)
{
	shell = thread_run(run_shell, NULL);
	return 0;
}

int usr_init(void)
{
	int ret;

	ret = usr_fs_init();
	if (ret)
		return ret;

	ret = usr_debug_init();
	if (ret)
		return ret;

	ret = usr_thread_init();
	if (ret)
		return ret;

	ret = mem_init();
	if (ret)
		return ret;

	start_new_shell();
	return 0;
}
