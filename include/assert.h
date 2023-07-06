#pragma once

#include <debug.h>
#include <stdio.h>

#define assert(_expr, ...)					\
	do {							\
		if (!(_expr)) {					\
			pr_err_debug("assert failed: ", ##__VA_ARGS__);\
			dump_stack();				\
		}						\
	} while (0)

#define warn_on(_expr, ...)					\
	do {							\
		if ((_expr)) {					\
			pr_err_debug("warning: ", ##__VA_ARGS__);\
			dump_stack();				\
		}						\
	} while (0)
