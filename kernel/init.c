#include <stdio.h>
#include <string.h>
#include <x86.h>
#include <graphic.h>
#include <memory.h>
#include <kmalloc.h>
#include <irq.h>
#include <rb_tree.h>
#include <debug.h>
#include <vmalloc.h>
#include <assert.h>
#include <schedule.h>
#include <fs.h>
#include <vector.h>
#include <usr.h>
#include <kernel.h>

#define MODULE "init"
#define MODULE_DEBUG 0

int kern_init(void) __attribute__((noreturn));

void reboot(void)
{
	pr_info("rebooting...");
	outb(0x92, 0x3);
}

void monitor(void)
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
}

int kernel_init_late(void)
{
	graphic_init();

	page_init_late();
	kmalloc_init_late();
	vmalloc_init_late();

	usr_init();
	return 0;
}

int kern_init(void)
{
	extern char edata[], end[];
	memset(edata, 0, end - edata);

	printk("\nloading...\n");

	kmalloc_early_init();

	debug_init();

	memory_init();

	stdio_init();

	irq_init();

	schedule_init();

	fs_init();

	pr_info("kernel init success!");

	kernel_init_late();

	while (1) {
		monitor();
	}
}
