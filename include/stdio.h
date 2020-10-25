#pragma once

#include <types.h>

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
void puts(const char *str);