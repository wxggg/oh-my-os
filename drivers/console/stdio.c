#include <stdio.h>

void putchar(int ch) { serial_putc(ch); }

void puts(const char *str)
{
    while (*str)
        putchar(*str++);
}