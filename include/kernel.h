#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdio.h>

#define offsetof __builtin_offsetof

#define container_of(ptr, type, member) ({			\
	void *__mptr = (void *)(ptr);				\
	((type *)(__mptr - offsetof(type, member))); })

#define assert(_expr)						\
	do {							\
		if (!(_expr)) {					\
			pr_err("assert failed");		\
			do {} while(1);				\
		}						\
	} while (0)

unsigned long tick();

#endif
