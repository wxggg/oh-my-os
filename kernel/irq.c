#include <asm-generic/mmu.h>
#include <x86.h>
#include <stdio.h>
#include <irq.h>
#include <error.h>
#include <debug.h>
#include <keyboard.h>

struct gate_desc {
	unsigned offset_15_0 : 16;
	unsigned selector : 16;
	unsigned args : 5;
	unsigned rsv : 3;
	unsigned type : 4;
	unsigned s : 1;
	unsigned dpl : 2;
	unsigned p : 1;
	unsigned offset_31_16 : 16;
};

static struct gate_desc idt_array[IRQ_NUM] = { 0 };
static struct pseudodesc idt_pd = {
	sizeof(idt_array) -1, (uintptr_t)idt_array
};

static irq_handler_t irq_handlers[IRQ_NUM];

static void set_gate(struct gate_desc *gate, unsigned long istrap,
		unsigned long selector, unsigned long offset, unsigned long dpl)
{
	gate->offset_15_0 = offset & 0xffff;
	gate->selector = selector;
	gate->args = 0;
	gate->rsv = 0;
	gate->type = istrap ? STS_TG32 : STS_IG32;
	gate->s = 0;
	gate->dpl = dpl;
	gate->p = 1;
	gate->offset_31_16 = (offset >> 16) & 0xffff;
}

void idt_init(void)
{
	extern uintptr_t __vectors[];

	for(int i = 0; i < 37; i++) {
		set_gate(&idt_array[i], 0, GD_KTEXT,
			 __vectors[i], DPL_KERNEL);
	}

	lidt(&idt_pd);
	pr_info("idt init success");
}

void irq_handler(struct trapframe *tf)
{
	if (tf->irq > IRQ_NUM) {
		pr_err("irq %d out of range", tf->irq);
		return;
	}

	if (irq_handlers[tf->irq]) {
		irq_handlers[tf->irq]();
		return;
	}

	switch (tf->irq)
	{
		case IRQ_PGFLT:
			pr_err("page fault, unable to access address:", hex(rcr2()));
			backtrace();
			while (1) {}
			break;
		default:
			pr_err("trap: unknown irq ", dec(tf->irq));
			backtrace();
			while (1) {}
			break;
	}
}

int request_irq(u16 irq, irq_handler_t fn)
{
	if (irq > IRQ_NUM)
		return -EINVAL;

	irq_handlers[irq] = fn;
	pr_info("register handler for irq ", dec(irq));
	return 0;
}

void irq_init(void)
{
	pic_init();
	idt_init();
	timer_init();
	keyboard_init();
	intr_enable();
}
