#pragma once

#include <types.h>

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

string *string_create(void);
void string_destroy(string *s);
int string_append_char(string *s, char c);
int string_append_strn(string *s, const char *str, size_t length);
int string_append_str(string *s, const char *str);
int string_append(string *s, string *a);
int string_append_int(string *s, int val, bool hex);
