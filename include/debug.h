#pragma once

#include <types.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_ENABLE
#define DEBUG_ENABLE_ALL 0

#define pr_info(...) printk_debug(MODULE, "info ", __VA_ARGS__, "\n")
#define pr_err(...) printk_debug(MODULE, "error", __VA_ARGS__, "\n")

#ifdef DEBUG_ENABLE
#define pr_debug(...)                                                     \
	do {                                                              \
		if (DEBUG_ENABLE_ALL || MODULE_DEBUG) {                   \
			printk_debug(MODULE, "debug", __VA_ARGS__, "\n"); \
		}                                                         \
	} while (0)
#else
#define pr_debug(...)
#endif

void dump_stack(void);
void dump_trapstack(uint32_t ebp, uint32_t eip);
void dmesg(void);
void debug_init(void);

#define BUG_ON(_expr, ...)                                              \
	do {                                                            \
		if (!(_expr)) {                                         \
			pr_err("bug on ", __FILE__, ":", __LINE__, ":", \
			       ##__VA_ARGS__);                          \
			dump_stack();                                   \
			halt();                                         \
		}                                                       \
	} while (0)

#define WARN_ON(_expr, ...)                                                 \
	do {                                                                \
		if ((_expr)) {                                              \
			pr_err("warning on ", __FILE__, ":", __LINE__, ":", \
			       ##__VA_ARGS__);                              \
			dump_stack();                                       \
		}                                                           \
	} while (0)
