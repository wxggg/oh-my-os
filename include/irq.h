#pragma once

#include <asm-generic/cpu.h>
#include <x86.h>
#include <types.h>

/* hardware irq */
#define PIC_TIMER 0
#define PIC_KEYBD 1
#define PIC_SLAVE 2
#define PIC_COM1 4

#define IRQ_GP 13
#define IRQ_PGFLT 14
#define IRQ_ERROR 19
#define IRQ_SPURIOUS 31
#define IRQ_OFFSET 32
#define IRQ_TIMER (IRQ_OFFSET + PIC_TIMER)
#define IRQ_KEYBD (IRQ_OFFSET + PIC_KEYBD)
#define IRQ_COM1 (IRQ_OFFSET + PIC_COM1)
#define IRQ_NUM 256

struct trapframe {
	/* pushal registers */
	struct {
		uint32_t edi;
		uint32_t esi;
		uint32_t ebp;
		uint32_t esp;
		uint32_t ebx;
		uint32_t edx;
		uint32_t ecx;
		uint32_t eax;
	} reg;

	uint16_t es;
	uint16_t padding1;
	uint16_t ds;
	uint16_t padding2;
	uint32_t irq;

	/* below here defined by x86 hardware */
	uint32_t err;
	uint32_t eip;
	uint16_t cs;
	uint16_t padding3;
	uint32_t eflags;

	/* below here only when crossing rings, such as from user to kernel */
	uint32_t esp;
	uint16_t ss;
	uint16_t padding4;
} __attribute__((packed));

typedef void (*irq_handler_t)(void);

int request_irq(u16 irq, irq_handler_t fn);
void idt_init(void);

void irq_init(void);
void pic_init(bool enable);
void pic_enable(unsigned int irq);

void timer_init(void);
#define TICK_NUM 100

static inline void intr_enable(void)
{
	sti();
}

static inline void intr_disable(void)
{
	cli();
}

struct rtc_date;

void ioapic_init(void);
void ioapic_enable(int irq, int cpu);

extern volatile int *lapic;
void lapic_init(void);
int lapic_id(void);
void lapic_eoi(void);
void lapic_startap(u8 apic_id, u32 addr);
void cmos_time(struct rtc_date *r);
