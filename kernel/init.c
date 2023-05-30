#include <stdio.h>
#include <string.h>
#include <x86.h>
#include <graphic.h>
#include <memory.h>
#include <kmalloc.h>
#include <irq.h>

int kern_init(void) __attribute__((noreturn));

int kern_init(void)
{
	extern char edata[], end[];
	memset(edata, 0, end - edata);

	kmalloc_early_init();

	serial_init();

	memory_init();

	pr_err("trap: pgfault, ", hex(rcr2()));

	irq_init();

	graphic_init();

	pr_info("kernel init success!");

	while (1) {
		halt();
	}
}
