#pragma once

#include <types.h>

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
int puts(const char *str);

int print(int count, ...);
int println(int count, ...);

#define printk(x) println(1, x)
