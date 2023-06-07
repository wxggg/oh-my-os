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

#define IO_TIMER     0x40
#define IO_TIMER_CMD 0x43

/**
 * Channel 0 is connected directly to IRQ0,
 * so it is best to use it only for purposes
 * that should generate interrupts.
 */
#define TIMER_CHANNEL0 0x00

/* mode 2: rate generate */
#define TIMER_MODE_RATEGEN (1 << 2)
#define TIMER_FREQ         1193182

/* r/w counter 16 bits */
#define TIMER_BIT_16 0x30
#define TIMER_OFFSET (1 << 8)

#define TIMER_DIV(x) ((TIMER_FREQ + (x) / 2) / (x))

volatile unsigned long ticks;

inline unsigned long tick()
{
	return ticks;
}

static void timer_irq_handler()
{
	ticks++;
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

    pic_enable(PIC_TIMER);
    request_irq(IRQ_TIMER, timer_irq_handler);
    pr_info("init timer success");
}
