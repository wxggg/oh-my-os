#pragma once

#include <debug.h>
#include <stdio.h>

#define assert_notrace(_expr, ...)				\
	do {							\
		if (!(_expr)) {					\
			pr_err("assert failed: ", ##__VA_ARGS__);\
			do {} while(1);				\
		}						\
	} while (0)

#define assert(_expr, ...)					\
	do {							\
		if (!(_expr)) {					\
			pr_err("assert failed: ", ##__VA_ARGS__);\
			backtrace();				\
			do {} while(1);				\
		}						\
	} while (0)

#define warn_on(_expr, ...)					\
	do {							\
		if ((_expr)) {					\
			pr_err("warning: ", ##__VA_ARGS__);	\
			backtrace();				\
		}						\
	} while (0)
