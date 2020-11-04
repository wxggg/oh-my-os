#include <stdio.h>
#include <string.h>
#include <x86.h>

int kern_init(void) __attribute__((noreturn));

int kern_init(void)
{
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    serial_init();

    printk("hello world!");

    while (1) {
        halt();
    }
}

