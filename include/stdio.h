#pragma once

#include <types.h>

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
int puts(const char *str);

int print(int count, ...);

#define printk(x) print(1, x)

