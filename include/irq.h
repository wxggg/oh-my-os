#pragma once

#include <asm-generic/cpu.h>
#include <x86.h>
#include <types.h>

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
void pic_init(void);
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
