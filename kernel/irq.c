#include <asm-generic/mmu.h>
#include <x86.h>
#include <stdio.h>
#include <irq.h>
#include <error.h>
#include <debug.h>
#include <keyboard.h>
#include <register.h>

#define MODULE "irq"
#define MODULE_DEBUG 0

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

	for(int i = 37; i < 256; i++) {
		set_gate(&idt_array[i], 0, GD_KTEXT,
			 __vectors[36], DPL_KERNEL);
	}

	lidt(&idt_pd);
	pr_info("idt init success");
}

static void dump_trapframe(struct trapframe *tf)
{
	pr_info("trapframe: ", hex(tf));
	pr_info("\teax:\t", hex(tf->reg.eax));
	pr_info("\tecx:\t", hex(tf->reg.ecx));
	pr_info("\tedx:\t", hex(tf->reg.edx));
	pr_info("\tebx:\t", hex(tf->reg.ebx));
	pr_info("\tesp:\t", hex(tf->reg.esp));
	pr_info("\tebp:\t", hex(tf->reg.ebp));
	pr_info("\tesi:\t", hex(tf->reg.esi));
	pr_info("\tedi:\t", hex(tf->reg.edi));
	pr_info("\tes:\t", hex(tf->es));
	pr_info("\tds:\t", hex(tf->ds));
	pr_info("\tirq:\t", dec(tf->irq));
	pr_info("\terr:\t", dec(tf->err));
	pr_info("\tcs:\t", hex(tf->cs));
	pr_info("\teip:\t", hex(tf->eip));
	pr_info("\teflags:\t", hex(tf->eflags));

	dump_trapstack(tf->reg.ebp, tf->eip);
}

void monitor(void);

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
		case IRQ_GP:
			pr_err("General protection fault");
			dump_trapframe(tf);
			monitor();
			break;
		case IRQ_PGFLT:
			pr_err("Unable to handle kernel NULL pointer "
			       "dereference at virtual address ", hex(rcr2()));
			dump_trapframe(tf);
			monitor();
			break;
		default:
			pr_err("trap: unknown irq ", dec(tf->irq));
			dump_trapframe(tf);
			monitor();
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
