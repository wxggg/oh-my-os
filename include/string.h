#pragma once

#include <types.h>
#include <vector.h>

typedef struct string {
	char *str;
	size_t length;
	size_t capacity;
} string;

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t len);

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t len);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

char *strchr(const char *s, char c);
char *strfind(const char *s, char c);
long strtol(const char *s, char **endptr, int base);

void *memset(void *s, char c, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int memcmp(const void *v1, const void *v2, size_t n);

void reverse_str(char *buf, int i, int j);
int to_hex(unsigned int val, char *buf, int len);
int to_str(int val, char *buf, int len);

static inline bool string_empty(string *s)
{
	return s->length == 0;
}

string *ksalloc(void);
void ksfree(string *s);
void ksinit(string *s, char *buf, size_t size);

extern const char ksappend_end[];
#define ksappend(s, ...) ksappend_args(s, ksappend_end, ##__VA_ARGS__, ksappend_end)

int ksappend_args(string *s, const char *end, ...);

int ksappend_str(string *s, const char *str);
int ksappend_char(string *s, char c);
char kspop_char(string *s);
int ksappend_strn(string *s, const char *str, size_t length);
int ksappend_str(string *s, const char *str);
int ksappend_int(string *s, int val);
int ksappend_hex(string *s, int val);

static inline int ksappend_kv(string *s, const char *key, int val)
{
	ksappend_str(s, key);
	ksappend_int(s, val);
	return 0;
}

static inline int ksappend_kvx(string *s, const char *key, int val)
{
	ksappend_str(s, key);
	ksappend_hex(s, val);
	return 0;
}

int kssplit(string *s, char c, vector *vec);
int ksfit(string *s, char c, int n);
int hex_to_value(string *s, int *value);
