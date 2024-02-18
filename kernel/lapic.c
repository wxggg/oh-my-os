// The local APIC manages internal (non-I/O) interrupts.
// See Chapter 8 & Appendix C of Intel processor manual volume 3.

#include <types.h>
#include <x86.h>
#include <irq.h>
#include <memory.h>
#include <kernel.h>
#include <string.h>
#include <debug.h>

#define MODULE "lapic"
#define MODULE_DEBUG 0

// Local APIC registers, divided by 4 for use as int[] indices.
#define ID (0x0020) // ID
#define VER (0x0030) // Version
#define TPR (0x0080) // Task Priority
#define EOI (0x00B0) // EOI
#define SVR (0x00F0) // Spurious Interrupt Vector
#define ENABLE 0x00000100 // Unit Enable
#define ESR (0x0280) // Error Status
#define ICRLO (0x0300) // Interrupt Command
#define INIT 0x00000500 // INIT/RESET
#define STARTUP 0x00000600 // Startup IPI
#define DELIVS 0x00001000 // Delivery status
#define ASSERT 0x00004000 // Assert interrupt (vs deassert)
#define DEASSERT 0x00000000
#define LEVEL 0x00008000 // Level triggered
#define BCAST 0x00080000 // Send to all APICs, including self.
#define BUSY 0x00001000
#define FIXED 0x00000000
#define ICRHI (0x0310) // Interrupt Command [63:32]
#define TIMER (0x0320) // Local Vector Table 0 (TIMER)
#define X1 0x0000000B // divide counts by 1
#define PERIODIC 0x00020000 // Periodic
#define PCINT (0x0340) // Performance Counter LVT
#define LINT0 (0x0350) // Local Vector Table 1 (LINT0)
#define LINT1 (0x0360) // Local Vector Table 2 (LINT1)
#define ERROR (0x0370) // Local Vector Table 3 (ERROR)
#define MASKED 0x00010000 // Interrupt masked
#define TICR (0x0380) // Timer Initial Count
#define TCCR (0x0390) // Timer Current Count
#define TDCR (0x03E0) // Timer Divide Configuration

volatile int *lapic; // Initialized in mp.c

#define lapic_addr(reg) ((u32 *)((u32)lapic + reg))

static u32 lapic_read(u32 reg)
{
	return *lapic_addr(reg);
}

static void lapic_write(u32 reg, u32 value)
{
	*lapic_addr(reg) = value;

	/* dummy read */
	lapic_read(ID);
}

void lapic_init(void)
{
	if (!lapic) {
		pr_err("lapic is not found");
		return;
	}

	kernel_map((uint32_t)lapic, (uint32_t)lapic, PAGE_SIZE, PTE_W);

	// Enable local APIC; set spurious interrupt vector.
	lapic_write(SVR, ENABLE | (IRQ_OFFSET + IRQ_SPURIOUS));

	// The timer repeatedly counts down at bus frequency
	// from TICR and then issues an interrupt.
	// If xv6 cared more about precise timekeeping,
	// TICR would be calibrated using an external time source.
	lapic_write(TDCR, X1);
	lapic_write(TIMER, PERIODIC | IRQ_TIMER);
	lapic_write(TICR, 10000000);

	// Disable logical interrupt lines.
	lapic_write(LINT0, MASKED);
	lapic_write(LINT1, MASKED);

	// Disable performance counter overflow interrupts
	// on machines that provide that interrupt entry.
	if (((lapic_read(VER) >> 16) & 0xFF) >= 4)
		lapic_write(PCINT, MASKED);

	// Map error interrupt to IRQ_ERROR.
	lapic_write(ERROR, IRQ_OFFSET + IRQ_ERROR);

	// Clear error status register (requires back-to-back writes).
	lapic_write(ESR, 0);
	lapic_write(ESR, 0);

	// Ack any outstanding interrupts.
	lapic_write(EOI, 0);

	// Send an Init Level De-Assert to synchronise arbitration ID's.
	lapic_write(ICRHI, 0);
	lapic_write(ICRLO, BCAST | INIT | LEVEL);
	while (lapic_read(ICRLO) & DELIVS)
		;

	// Enable interrupts on the APIC (but not on the processor).
	lapic_write(TPR, 0);
}

int lapic_id(void)
{
	if (!lapic)
		return 0;
	return lapic_read(ID) >> 24;
}

// Acknowledge interrupt.
void lapic_eoi(void)
{
	if (lapic)
		lapic_write(EOI, 0);
}

// Spin for a given number of microseconds.
// On real hardware would want to tune this dynamically.
void micro_delay(int us)
{
}

#define CMOS_PORT 0x70
#define CMOS_RETURN 0x71

// Start additional processor running entry code at addr.
// See Appendix B of MultiProcessor Specification.
void lapic_startap(u8 apic_id, u32 addr)
{
	int i;
	u16 *wrv;

	// "The BSP must initialize CMOS shutdown code to 0AH
	// and the warm reset vector (DWORD based at 40:67) to point at
	// the AP startup code prior to the [universal startup algorithm]."
	outb(CMOS_PORT, 0xF); // offset 0xF is shutdown code
	outb(CMOS_PORT + 1, 0x0A);
	wrv = (u16 *)phys_to_virt((0x40 << 4 | 0x67)); // Warm reset vector
	wrv[0] = 0;
	wrv[1] = addr >> 4;

	// "Universal startup algorithm."
	// Send INIT (level-triggered) interrupt to reset other CPU.
	lapic_write(ICRHI, apic_id << 24);
	lapic_write(ICRLO, INIT | LEVEL | ASSERT);
	micro_delay(200);
	lapic_write(ICRLO, INIT | LEVEL);
	micro_delay(100); // should be 10ms, but too slow in Bochs!

	// Send startup IPI (twice!) to enter code.
	// Regular hardware is supposed to only accept a STARTUP
	// when it is in the halted state due to an INIT.  So the second
	// should be ignored, but it is part of the official Intel algorithm.
	// Bochs complains about the second one.  Too bad for Bochs.
	for (i = 0; i < 2; i++) {
		lapic_write(ICRHI, apic_id << 24);
		lapic_write(ICRLO, STARTUP | (addr >> 12));
		micro_delay(200);
	}
}

#define CMOS_STATA 0x0a
#define CMOS_STATB 0x0b
#define CMOS_UIP (1 << 7) // RTC update in progress

#define SECS 0x00
#define MINS 0x02
#define HOURS 0x04
#define DAY 0x07
#define MONTH 0x08
#define YEAR 0x09

static u32 cmos_read(u32 reg)
{
	outb(CMOS_PORT, reg);
	micro_delay(200);

	return inb(CMOS_RETURN);
}

static void fill_rtcdate(struct rtc_date *r)
{
	r->second = cmos_read(SECS);
	r->minute = cmos_read(MINS);
	r->hour = cmos_read(HOURS);
	r->day = cmos_read(DAY);
	r->month = cmos_read(MONTH);
	r->year = cmos_read(YEAR);
}

// qemu seems to use 24-hour GWT and the values are BCD encoded
void cmos_time(struct rtc_date *r)
{
	struct rtc_date t1, t2;
	int sb, bcd;

	sb = cmos_read(CMOS_STATB);

	bcd = (sb & (1 << 2)) == 0;

	// make sure CMOS doesn't modify time while we read it
	for (;;) {
		fill_rtcdate(&t1);
		if (cmos_read(CMOS_STATA) & CMOS_UIP)
			continue;
		fill_rtcdate(&t2);
		if (memcmp(&t1, &t2, sizeof(t1)) == 0)
			break;
	}

	// convert
	if (bcd) {
#define CONV(x) (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
		CONV(second);
		CONV(minute);
		CONV(hour);
		CONV(day);
		CONV(month);
		CONV(year);
#undef CONV
	}

	*r = t1;
	r->year += 2000;
}
