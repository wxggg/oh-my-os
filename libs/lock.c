#include <lock.h>
#include <x86.h>
#include <debug.h>
#include <assert.h>

#define MODULE "lock"
#define MODULE_DEBUG 0

void spin_lock(spinlock_t *lock)
{
	int count = 0;

	do {
		while (atomic_read(lock) != 0) {
			cpu_relax();

			assert(count++ < 10000000);
		}
	} while (cmpxchg(&lock->counter, 0, 1));
}

