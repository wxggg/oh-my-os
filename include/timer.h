#pragma once

#include <types.h>
#include <list.h>

struct time {
        uint32_t hours;
        uint32_t minutes;
        uint32_t seconds;
        uint32_t msecs;
        uint32_t ticks;
};

struct thread;

struct timer {
        struct time expired;
        struct thread *thread;
        struct list_node node;
};

int time_now(struct time *t);
uint64_t time_ms(void);
struct timer *timer_create(struct thread *thread, struct time *time_expired);
void sleep(int seconds);
void msleep(int msecs);