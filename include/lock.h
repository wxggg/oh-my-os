#pragma once

#include <atomic.h>

typedef atomic_t spinlock_t;

static inline void spinlock_init(spinlock_t *lock)
{
	lock->counter = 0;
}

void spin_lock(spinlock_t *lock);

static inline void spin_unlock(spinlock_t *lock)
{
	atomic_set(lock, 0);
}
