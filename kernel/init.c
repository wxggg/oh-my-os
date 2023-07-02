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

void monitor(void)
{
	string *s = string_create();
	while (1) {
		readline(s);

		if (!s->str)
			continue;

		if (!strcmp(s->str, "backtrace")) {
			backtrace();
		}

		if (!strcmp(s->str, "gpu_dump")) {
			gpu_dump();
		}

		if (!strcmp(s->str, "vma_dump")) {
			vma_dump();
		}

		if (!strcmp(s->str, "kmalloc_dump")) {
			kmalloc_dump();
		}

		if (!strcmp(s->str, "page_dump")) {
			page_dump();
		}
	}
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

	while (1) {
		monitor();
	}
}
