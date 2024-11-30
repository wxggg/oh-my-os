/*
 * Intel 8253 Programmable Interval Timer (PIT)
 * perform timeing and counting functions using three 16-bit counters
 *
 * https://en.wikipedia.org/wiki/Intel_8253
 * https://wiki.osdev.org/Programmable_Interval_Timer
 *
 * I/O port     Usage
 * 0x40         Channel 0 data port (read/write)
 * 0x41         Channel 1 data port (read/write)
 * 0x42         Channel 2 data port (read/write)
 * 0x43         Mode/Command register (write only, a read is ignored)
 */

#include <irq.h>
#include <stdio.h>
#include <x86.h>
#include <schedule.h>
#include <debug.h>
#include <smp.h>
#include <kernel.h>
#include <timer.h>
#include <kmalloc.h>

#define MODULE "timer"
#define MODULE_DEBUG 0

#define IO_TIMER 0x40
#define IO_TIMER_CMD 0x43

/**
 * Channel 0 is connected directly to IRQ0,
 * so it is best to use it only for purposes
 * that should generate interrupts.
 */
#define TIMER_CHANNEL0 0x00

/* mode 2: rate generate */
#define TIMER_MODE_RATEGEN (1 << 2)
#define TIMER_FREQ 1193182

/* r/w counter 16 bits */
#define TIMER_BIT_16 0x30
#define TIMER_OFFSET (1 << 8)

#define TIMER_DIV(x) ((TIMER_FREQ + (x) / 2) / (x))

static struct time times[MAX_CPU];
static struct list_node timer_lists[MAX_CPU];

static bool time_cmp(struct time *t1, struct time *t2)
{
	if (t1->hours > t2->hours)
		return true;
	if (t1->hours < t2->hours)
		return false;

	if (t1->minutes > t2->minutes)
		return true;
	if (t1->minutes < t2->minutes)
		return false;

	if (t1->seconds * 1000 + t1->msecs >= t2->seconds * 1000 + t2->msecs)
		return true;

	return false;
}

static void time_add_ms(struct time *t, int msecs)
{
	t->msecs += msecs;
	t->seconds += t->msecs / 1000;
	t->msecs = t->msecs % 1000;

	t->minutes += t->seconds / 60;
	t->seconds = t->seconds % 60;

	t->hours += t->minutes / 60;
	t->minutes = t->minutes % 60;
}

static void time_add(struct time *t, int seconds)
{
	time_add_ms(t, seconds * 1000);
}

static void timer_irq_handler()
{
	struct time *t = &times[cpu_id()];
	struct list_node *node;
	struct timer *timer;

	t->ticks++;
	t->msecs = (t->ticks % 100) * 10;

	/* 1s = 100 ticks = 1000ms*/
	if (t->ticks % TICK_NUM == 0)
		time_add(t, 1);

	if (t->ticks == 0) {
		/* find timerout timer */
		node = list_next(&timer_lists[cpu_id()]);
		while (node) {
			timer = container_of(node, struct timer, node);
			if (time_cmp(&timer->expired, t))
				thread_wakeup(timer->thread);

			node = node->next;
		}

		schedule();
	}
}

/**
 * timer_init - init 8253 pit
 * generate 100 timer interrupt per second
 */
void timer_init(void)
{
	outb(IO_TIMER_CMD, TIMER_CHANNEL0 | TIMER_MODE_RATEGEN | TIMER_BIT_16);

	outb(IO_TIMER, TIMER_DIV(TICK_NUM) % TIMER_OFFSET);
	outb(IO_TIMER, TIMER_DIV(TICK_NUM) / TIMER_OFFSET);

	/* pic_enable(PIC_TIMER); */
	request_irq(IRQ_TIMER, timer_irq_handler);

	list_init(&timer_lists[cpu_id()]);

	pr_info("init timer success");
}

int time_now(struct time *t)
{
	memcpy(t, &times[cpu_id()], sizeof(struct time));
	return 0;
}

uint64_t time_ms(void)
{
	struct time *t = &times[cpu_id()];
	return ((t->hours * 60 + t->minutes) * 60 + t->seconds) * 1000 +
	       t->msecs;
}

struct timer *timer_create(struct thread *thread, struct time *time_expired)
{
	struct timer *t;

	t = kmalloc(sizeof(*t));
	if (!t)
		return NULL;

	t->expired = *time_expired;
	t->thread = thread;
	list_init(&t->node);
	return t;
}

void sleep(int seconds)
{
	msleep(seconds * 1000);
}

void msleep(int msecs)
{
	struct timer timer;

	time_now(&timer.expired);
	time_add_ms(&timer.expired, msecs);
	timer.thread = current;
	current->state = THREAD_SLEEPING;

	list_insert_before(&timer.node, &timer_lists[cpu_id()]);
	schedule();
	list_remove(&timer.node);
}