#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define FORMATBUF_NR 	16
#define FORMATBUF_SIZE 	32

static char formatbuf[FORMATBUF_NR][FORMATBUF_SIZE];

static int formatbuf_i = 0;

static char * get_format_buffer()
{
	char * buf = &formatbuf[formatbuf_i][0];
	formatbuf_i = (formatbuf_i + 1) % FORMATBUF_SIZE;
	return buf;
}

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
int print(uint16_t count, ...)
{
    va_list args;
    int r = 0;

	if (count > FORMATBUF_NR) {
		puts("print error count ");
		puts(istr(count));
		puts("\n");
		return -1;
	}

    va_start(args, count);
    for (size_t i = 0; i < count; i++) {
        r += puts(va_arg(args, char *));
    }
    va_end(args);

    return r + 1;
}

/**
 * print - print several strings
 * - only support string for every parameters
 * - will end with '\n' automatically
 */
int println(uint16_t count, ...)
{
    va_list args;
    int r = 0;

	if (count > FORMATBUF_NR) {
		puts("print error count ");
		puts(istr(count));
		puts("\n");
		return -1;
	}

    va_start(args, count);
    for (size_t i = 0; i < count; i++) {
        r += puts(va_arg(args, char *));
    }
    va_end(args);

    puts("\n");
    return r + 1;
}

/**
 * istr - format value to string
 * - notice that the buffer for the string is
 *   temprarily used.
 */
const char * istr(int val)
{
	char *buf = get_format_buffer();
	to_str(val, buf, FORMATBUF_SIZE);
	return buf;
}

/**
 * xstr - format value to hex string
 * - notice that the buffer for the string is
 *   temprarily used.
 */
const char * xstr(unsigned int val)
{
	char *buf = get_format_buffer();
	to_hex(val, buf, FORMATBUF_SIZE);
	return buf;
}
