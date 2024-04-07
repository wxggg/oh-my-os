#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <queue.h>
#include <lock.h>

#define STDIO_MAX_ARGS 128

static string g_out_string[STDIO_MAX_ARGS];
static char g_out_buf[STDIO_MAX_ARGS][128];
static int index = 0;

const char line_end[1];

queue *stdio_que;

spinlock_t pr_lock;

string *get_out_string(void)
{
	string *s;
	index = (index + 1) % STDIO_MAX_ARGS;
	s = &g_out_string[index];
	ksinit(s, &g_out_buf[index][0], 128);
	return s;
}

void putchar(int ch)
{
	serial_putc(ch);
}

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

	spin_lock(&pr_lock);

	va_start(args, end);
	while (p != end && n <= STDIO_MAX_ARGS) {
		p = va_arg(args, char *);
		ret += puts(p);
		n++;
	}
	va_end(args);

	spin_unlock(&pr_lock);
	return ret;
}

char readchar(void)
{
	while (1) {
		if (serial_received())
			return serial_getc();

		if (!queue_empty(stdio_que))
			return dequeue(stdio_que, char);

		halt();
	}
}

void readline(string *s)
{
	char c;
	s->length = 0;

	printk("~/oh-my-os <main*> # ");

	while (1) {
		c = readchar();
		printk(ch(c));
		if (c == '\n')
			return;

		ksappend_char(s, c);
	}
}

void stdio_init(void)
{
	stdio_que = queue_create(char);

	serial_init();
	spinlock_init(&pr_lock);
}
