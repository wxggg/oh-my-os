#pragma once

#include <x86.h>

#define offsetof __builtin_offsetof

#define container_of(ptr, type, member) ({			\
	void *__mptr = (void *)(ptr);				\
	((type *)(__mptr - offsetof(type, member))); })

unsigned long tick();

void reboot(void);
