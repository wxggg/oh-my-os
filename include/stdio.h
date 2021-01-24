#pragma once

#include <types.h>

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
int puts(const char *str);

int print(uint16_t count, ...);
int println(uint16_t count, ...);

#define printk(x) println(1, x)

const char * istr(int val);
const char * xstr(unsigned int val);
