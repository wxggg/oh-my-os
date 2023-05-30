#pragma once

#include <types.h>
#include <string.h>

#define PR_DEBUG

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
int puts(const char *str);

extern const char line_end[1];
int print_args(const char *end, ...);
#define printk(...)  print_args(line_end, ##__VA_ARGS__, line_end)
#define pr_info(...) printk("[kernel  info]: ", __VA_ARGS__, "\n")
#define pr_err(...)  printk("[kernel error]: ", __FILE__, " ", __func__, ": ", __VA_ARGS__, "\n")

#ifdef PR_DEBUG
#define pr_debug(...)  printk("[kernel debug]: ", __FILE__, " ", __func__, ": ", __VA_ARGS__, "\n")
#else
#define pr_debug(...)
#endif

string *get_out_string(void);

static inline char *__dec(long val)
{
	string *s = get_out_string();
	string_append_int(s, val, false);
	return s->str;
}

static inline char *__hex(unsigned long val)
{
	string *s = get_out_string();
	string_append_int(s, val, true);
	return s->str;
}

static inline char *__range(unsigned long start, unsigned long end)
{
	string *s = get_out_string();
	string_append_char(s, '<');
	string_append_int(s, start, true);
	string_append_char(s, ',');
	string_append_char(s, ' ');
	string_append_int(s, end, true);
	string_append_char(s, '>');
	return s->str;
}

#define dec(val) __dec((int)val)
#define hex(val) __hex((unsigned long)val)
#define range(start, end) __range((unsigned long)start, (unsigned long)end)
