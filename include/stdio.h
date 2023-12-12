#pragma once

#include <types.h>
#include <string.h>
#include <kernel.h>
#include <queue.h>

int serial_received();
void serial_init(void);
void serial_putc(int ch);
char serial_getc(void);

void putchar(int ch);
int puts(const char *str);

extern queue *stdio_que;

char readchar(void);
void readline(string *s);

void stdio_init(void);

extern const char line_end[];
int print_args(const char *end, ...);
int print_debug(const char *module, const char *debug, const char *end, ...);

#define printk(...) print_args(line_end, ##__VA_ARGS__, line_end)
#define printk_debug(module, debug, ...) \
	print_debug(module, debug, line_end, ##__VA_ARGS__, line_end)

string *get_out_string(void);

static inline char *ch(char c)
{
	string *s = get_out_string();
	ksappend_char(s, c);
	return s->str;
}

static inline char *__dec(long val)
{
	string *s = get_out_string();
	ksappend_int(s, val);
	return s->str;
}

static inline char *__hex(unsigned long val)
{
	string *s = get_out_string();
	ksappend_hex(s, val);
	return s->str;
}

static inline char *__pair(unsigned long a, unsigned long b)
{
	string *s = get_out_string();
	ksappend(s, "<", __hex(a), ", ", __hex(b), ">");
	return s->str;
}

static inline char *__repeat(const char *str, unsigned long n)
{
	string *s = get_out_string();

	while (n--)
		ksappend_str(s, str);

	return s->str;
}

#define dec(val) __dec((int)val)
#define hex(val) __hex((unsigned long)val)
#define pair(a, b) __pair((unsigned long)a, (unsigned long)b)
#define range(start, end) pair(start, end)
#define repeat(str, n) __repeat((const char *)str, (unsigned long)n)
