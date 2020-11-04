#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void putchar(int ch) { serial_putc(ch); }

int puts(const char *str)
{
    int r = 0;

    if (!str)
        return 0;

    while (*str) {
        putchar(*str++);
        r++;
    }

    return r;
}

/**
 * print - print several strings
 * - only support string for every parameters
 * - will end with '\n' automatically
 */
int print(int count, ...)
{
    va_list args;
    int r = 0;

    va_start(args, count);
    for (size_t i = 0; i < count; i++) {
        r += puts(va_arg(args, char *));
    }
    va_end(args);

    puts("\n");
    return r + 1;
}

