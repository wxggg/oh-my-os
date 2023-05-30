#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define STDIO_MAX_ARGS 	32

static string g_out_string[STDIO_MAX_ARGS];
static int index = 0;

const char line_end[1];

string *get_out_string(void)
{
	string *s;
	index = (index + 1) % STDIO_MAX_ARGS;
	s = &g_out_string[index];
	s->length = 0;
	return s;
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
