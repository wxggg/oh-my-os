/*
 * Serial communication
 * - The serial and parallel port denotes two kinds of port communication
 * methods. Since parallel port is nowadays rarely used, only serial port is
 * supported here.
 * https://en.wikipedia.org/wiki/Serial_port
 * https://wiki.osdev.org/Serial_Ports
 */

#include <stdio.h>
#include <x86.h>

#define COM1 0x3F8

#define TX  0 /* Out: Transmit buffer (DLAB=0) */
#define IER 1 /* Out: Interrupt Enable Register */
#define FCR 2 /* Out: FIFO Control Register */
#define LCR 3 /* Out: Line Control Register */
#define MCR 4 /* Out: Modem Control Register */

#define LCR_DLL 0 /* Out: Divisor Latch Low (DLAB=1) */
#define LCR_DLH 1 /* Out: Divisor Latch High (DLAB=1) */

#define RX  0 /* In:  Receive buffer (DLAB=0) */
#define IIR 2 /* In:  Interrupt ID Register */
#define LSR 5 /* In:  Line Status Register */

#define LSR_RXRDY 0x01
#define LSR_TXRDY 0x20 /* Transmit buffer ready */

static bool serial_exists = 0;

static void serial_out(uint16_t offset, uint8_t value)
{
	outb(COM1 + offset, value);
}

static uint8_t serial_in(uint16_t offset) { return inb(COM1 + offset); }

void serial_init(void)
{
	serial_out(FCR, 0); /* Turn off the FIFO */

	/* Set speed; requires DLAB latch */
	serial_out(LCR, 0x80);     /* Enable DLAB (set baud rate divisor */
	serial_out(LCR_DLL, 0x03); /* Set divisor to 3 (lo byte) 38400 baud */
	serial_out(LCR_DLH, 0x00); /* Set divisor to 0 (hi byte) */
	/* 8 data bits, 1 stop bit, parity off; turn off DLAB latch */
	serial_out(LCR, 0x03 & ~0x80);

	/* No modem controls */
	serial_out(MCR, 0x00);

	/* Enable rcv interrupts */
	serial_out(IER, 0x01); /* Enable receiver data interrupt */

	/*
	 * Clear any preexisting overrun indications and interrupts
	 * Serial port doesn't exist if LSR returns 0xFF
	 */
	serial_exists = (serial_in(LSR) != 0xFF);
	(void)serial_in(IIR);
	(void)serial_in(RX);
}

int serial_received()
{
	return serial_in(LSR) & LSR_RXRDY;
}

static int is_transmit_empty()
{
	return serial_in(LSR) & LSR_TXRDY;
}

/* stupid I/O delay routine necessitated by historical PC design flaws */
static void delay(void)
{
	inb(0x84);
	inb(0x84);
	inb(0x84);
	inb(0x84);
}

/*
 * serial_putc
 * print character to serial port
 */
void serial_putc(int c)
{
	while (!is_transmit_empty())
		delay();

	serial_out(TX, c);
}

/**
 * serial_getc
 * get character from serial port
 */
char serial_getc(void)
{
	char c;

	c = serial_in(RX);
	if (c == 13)
		return '\n';
	return c;
}
