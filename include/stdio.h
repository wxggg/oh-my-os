#pragma once

#include <types.h>

void serial_init(void);
void serial_putc(int ch);

void putchar(int ch);
int puts(const char *str);

char * get_format_buffer();
size_t get_format_size();

extern const char line_end[1];
int print_args(const char *end, ...);
#define printk(...)  print_args(line_end, ##__VA_ARGS__, line_end)
#define pr_info(...) printk("[kernel info]: ", __FILE__, " ", __VA_ARGS__, "\n")

const char * dstr(int val);
const char * xstr(unsigned int val);
