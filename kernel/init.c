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

int kern_init(void) __attribute__((noreturn));

void reboot(void)
{
	pr_info("rebooting...");
	outb(0x92, 0x3);
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

	graphic_init();

	pr_info("kernel init success!");

	vmalloc(1024 * PAGE_SIZE);

	string *s = string_create();
	while (1) {
		readline(s);

		if (!strcmp(s->str, "backtrace")) {
			backtrace();
		}
	}
}
