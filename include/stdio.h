#pragma once

#include <types.h>
#include <string.h>
#include <kernel.h>

#define PR_DEBUG

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
int puts(const char *str);

extern const char line_end[1];
int print_args(const char *end, ...);
#define printk(...)  print_args(line_end, ##__VA_ARGS__, line_end)
#define printk_tick(...)  printk("[", dec(tick() / 100), ".", dec(tick() % 100), "] ", __VA_ARGS__)
#define pr_info(...) printk_tick(__VA_ARGS__, "\n")
#define pr_err(...)  printk_tick("[error] ", __FILE__, " ", __func__, ": ", __VA_ARGS__, "\n")

#ifdef PR_DEBUG
#define pr_debug(...)  printk_tick("[debug] ", __FILE__, " ", __func__, ": ", dec(__LINE__), ":"  __VA_ARGS__, "\n")
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

static inline char *__pair(unsigned long a, unsigned long b)
{
	string *s = get_out_string();
	string_append_char(s, '<');
	string_append_int(s, a, true);
	string_append_char(s, ',');
	string_append_char(s, ' ');
	string_append_int(s, b, true);
	string_append_char(s, '>');
	return s->str;
}

static inline char *__repeat(const char *str, unsigned long n)
{
	string *s = get_out_string();

	while (n--)
		string_append_str(s, str);

	return s->str;
}

#define dec(val) __dec((int)val)
#define hex(val) __hex((unsigned long)val)
#define pair(a, b) __pair((unsigned long)a, (unsigned long)b)
#define range(start, end) pair(start, end)
#define repeat(str, n) __repeat((const char *)str, (unsigned long)n)
