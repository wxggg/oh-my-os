/*
 * The 8259 PIC driver
 * Master PIC Command I/O port 0x0020, Data I/O port 0x0021
 * Slave PIC Command I/O port 0x00A0, Data I/O port 0x00A1
 *
 * https://wiki.osdev.org/8259_PIC
 */

#include <interrupt.h>
#include <stdio.h>
#include <types.h>
#include <x86.h>

#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1

struct pic {
    bool init;
    uint16_t mask;
    struct pic_ops *ops;
};

struct pic init_pic;
struct pic *pic = NULL;

static void pic_setmask(uint16_t mask)
{
    pic->mask = mask;
    outb(PIC_MASTER_DATA, pic->mask);
    outb(PIC_SLAVE_DATA, pic->mask >> 8);
}

static void master_init(void)
{
    /*
     * ICW1: 0b0001g0hi
     * g:  0 = edge triggering, 1 = level triggering
     * h:  0 = cascaded PICs, 1 = master only
     * i:  0 = no ICW4, 1 = ICW4 required
     */
    outb(PIC_MASTER_CMD, 0x11);

    /*
     * ICW2: set interrupt irq offset as 0x20
     * map master 8259A irq 0-7 to 0x20-0x27
     */
    outb(PIC_MASTER_DATA, IRQ_OFFSET);

    /*
     * ICW3: cascade
     * 0x4(0b100) means the 2th ir line connect to slave
     */
    outb(PIC_MASTER_DATA, 1 << IRQ_SLAVE);

    /*
     * ICW4: 0x3 means automatic EOF
     */
    outb(PIC_MASTER_DATA, 0x3);
}

static void slave_init(void)
{
    /* ICW1 */
    outb(PIC_SLAVE_CMD, 0x11);

    /* ICW2 */
    outb(PIC_SLAVE_DATA, IRQ_OFFSET + 8);

    /* ICW3 */
    outb(PIC_SLAVE_DATA, IRQ_SLAVE);

    /* ICW4 */
    outb(PIC_SLAVE_DATA, 0x3);
}

void pic_init(void)
{
    uint16_t irq_mask;

    outb(PIC_MASTER_DATA, 0xFF);
    outb(PIC_SLAVE_DATA, 0xFF);

    master_init();

    slave_init();

    /*
     * set OCW2 of master and slave
     * ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
     *  p:  0 = no polling, 1 = polling mode
     * rs:  0x = NOP, 10 = read IRR, 11 = read ISR
     */

    /* clear specific mask */
    outb(PIC_MASTER_CMD, 0x68);
    /* read IRR by default */
    outb(PIC_MASTER_CMD, 0x0a);

    outb(PIC_SLAVE_CMD, 0x68);
    outb(PIC_SLAVE_CMD, 0x0a);

    /* enable all irq of master */
    irq_mask = 0xFFFF & ~(1 << IRQ_SLAVE);
    if (irq_mask != 0xFFFF) {
        pic_setmask(irq_mask);
    }

    printk("pic_init finished\n");
}

void pic_enable(unsigned int irq) { pic_setmask(pic->mask & ~(1 << irq)); }
