#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define STDIO_MAX_ARGS 	32
#define FORMATBUF_SIZE 	32

static char formatbuf[STDIO_MAX_ARGS][FORMATBUF_SIZE];

static int formatbuf_i = 0;

const char line_end[1];

char * get_format_buffer()
{
	char * buf = &formatbuf[formatbuf_i][0];
	formatbuf_i = (formatbuf_i + 1) % STDIO_MAX_ARGS;
	return buf;
}

size_t get_format_size()
{
	return FORMATBUF_SIZE;
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
 * print - print all args as string, check last string as end
 *
 * @end: the string used to check end
 */
int print_args(const char *end, ...)
{
	char *p = NULL;
	va_list args;
	int n = 0;
	int ret = 0;

	va_start(args, end);
	while (p != end && n <= STDIO_MAX_ARGS) {
		p = va_arg(args, char *);
		ret += puts(p);
		n++;
	}
	va_end(args);

	return ret;
}

/**
 * dstr - format value to string
 * - notice that the buffer for the string is
 *   temprarily used.
 */
const char * dstr(int val)
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
	buf[0] = '0';
	buf[1] = 'x';
	to_hex(val, buf + 2, FORMATBUF_SIZE - 2);
	return buf;
}
