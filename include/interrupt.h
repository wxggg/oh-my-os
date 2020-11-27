
/* hardware irq */
#define IRQ_TIMER		0
#define IRQ_KEYBOARD	1
#define IRQ_SLAVE       2

#define IRQ_OFFSET		0x20

void pic_init(void);

void pic_enable(unsigned int irq);
