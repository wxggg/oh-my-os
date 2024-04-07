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
#include <smp.h>
#include <lock.h>

bool os_start = false;

#define MODULE "init"
#define MODULE_DEBUG 0

void start_kernel(void) __attribute__((noreturn));

void reboot(void)
{
	pr_info("rebooting...");
	outb(0x92, 0x3);
}

int kernel_init_late(void)
{
	graphic_init();

	page_init_late();
	kmalloc_init_late();
	vmalloc_init_late();
	smp_init_late();
	return 0;
}

void start_kernel(void)
{
	extern char edata[], end[];
	memset(edata, 0, end - edata);

	printk("\nloading...\n");

	kmalloc_early_init();

	debug_init();

	memory_init();

	stdio_init();

	smp_init();

	irq_init();

	schedule_init();

	fs_init();

	kernel_init_late();

	usr_init();

	this_cpu()->started = true;
	cpu_up(1);

	pr_info("kernel init success!");

	os_start = true;

	while (1) {
		schedule();
	}
}
