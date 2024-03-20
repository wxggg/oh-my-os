#pragma once

#include <x86.h>

#define offsetof __builtin_offsetof

#define container_of(ptr, type, member)                      \
	({                                                   \
		void *__mptr = (void *)(ptr);                \
		((type *)(__mptr - offsetof(type, member))); \
	})

struct rtc_date {
	u32 second;
	u32 minute;
	u32 hour;
	u32 day;
	u32 month;
	u32 year;
};

unsigned long tick();

void reboot(void);

extern bool os_start;
