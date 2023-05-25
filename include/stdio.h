#pragma once

#include <types.h>
#include <string.h>

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
int puts(const char *str);

extern const char line_end[1];
int print_args(const char *end, ...);
#define printk(...)  print_args(line_end, ##__VA_ARGS__, line_end)
#define pr_info(...) printk("[kernel  info]: ", __FILE__, " ", __VA_ARGS__, "\n")
#define pr_err(...)  printk("[kernel error]: ", __FILE__, " ", __VA_ARGS__, "\n")

string *get_out_string(void);

static inline char *dec(int val)
{
	string *s = get_out_string();
	string_append_int(s, val, false);
	string_append_char(s, ' ');
	return s->str;
}

static inline char *hex(unsigned int val)
{
	string *s = get_out_string();
	string_append_int(s, val, true);
	string_append_char(s, ' ');
	return s->str;
}

static inline char *range(unsigned int start, unsigned int end)
{
	string *s = get_out_string();
	string_append_char(s, '<');
	string_append_int(s, start, true);
	string_append_char(s, ',');
	string_append_char(s, ' ');
	string_append_int(s, end, true);
	string_append_char(s, '>');
	string_append_char(s, ' ');
	return s->str;
}
