#include <types.h>
#include <x86.h>
#include <debug.h>
#include <irq.h>

#define MODULE "PIC"
#define MODULE_DEBUG 0

#define ICW1_ICW4		0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE		0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL		0x08		/* Level triggered (edge) mode */
#define ICW1_INIT		0x10		/* Initialization - required! */

#define ICW4_8086		0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO		0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM		0x10		/* Special fully nested (not) */

#define IO_PIC1		0x20 //Master
#define IO_PIC2 	0xA0 //Slave

static void pic_write(uint16_t port, uint16_t val)
{
	outb(port, val);
}

static void pic_master_write_cmd(uint16_t cmd)
{
	pic_write(IO_PIC1, cmd);
}

static void pic_master_write_data(uint16_t data)
{
	pic_write(IO_PIC1 + 1, data);
}

static void pic_slave_write_cmd(uint16_t cmd)
{
	pic_write(IO_PIC2, cmd);
}

static void pic_slave_write_data(uint16_t data)
{
	pic_write(IO_PIC2 + 1, data);
}

static bool __pic_init = 0;
static uint16_t irq_mask = 0xFFFF & ~(1 << PIC_SLAVE);

static void pic_setmask(uint16_t mask)
{
	if (!__pic_init) return;

	pic_master_write_data(mask);
	pic_slave_write_data(mask >> 8);
}

void pic_enable(unsigned int irq)
{
	irq_mask &= ~(1 << irq);
	pic_setmask(irq_mask);
}

void pic_init(void)
{
	__pic_init = 1;

	pic_master_write_data(0xFF);
	pic_master_write_cmd(ICW1_INIT | ICW1_ICW4);
	pic_master_write_data(IRQ_OFFSET);
	pic_master_write_data(1 << PIC_SLAVE);
	pic_master_write_data(ICW4_8086 | ICW4_AUTO);

	pic_slave_write_data(0xFF);
	pic_slave_write_cmd(ICW1_INIT | ICW1_ICW4);
	pic_slave_write_data(IRQ_OFFSET + 8);
	pic_slave_write_data(PIC_SLAVE);
	pic_slave_write_data(ICW4_8086 | ICW4_AUTO);

	if(irq_mask != 0xFFFF) {
		pic_setmask(irq_mask);
	}

	pr_info("pic init finished");
}
