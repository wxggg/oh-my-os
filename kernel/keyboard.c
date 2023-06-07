#include <types.h>
#include <x86.h>
#include <keyboard.h>
#include <stdio.h>
#include <irq.h>
#include <kernel.h>

#define NO 0
#define SHIFT (1 << 0)
#define CTL (1 << 1)
#define ALT (1 << 2)
#define CAPSLOCK (1 << 3)
#define NUMLOCK (1 << 4)
#define SCROLLLOCK (1 << 5)
#define E0ESC (1 << 6)

static uint8_t shiftcode[256] = {
	[0x1D] = CTL,
	[0x2A] = SHIFT,
	[0x36] = SHIFT,
	[0x38] = ALT,
	[0x9D] = CTL,
	[0xB8] = ALT
};

static uint8_t togglecode[256] = {
	[0x3A] = CAPSLOCK,
	[0x45] = NUMLOCK,
	[0x46] = SCROLLLOCK
};

static uint8_t normalmap[256] = {
	NO, 0x1B, '1', '2', '3', '4', '5', '6', // 0x00
	'7', '8', '9', '0', '-', '=', '\b', '\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', // 0x10
	'o', 'p', '[', ']', '\n', NO, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 0x20
	'\'', '`', NO, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', NO, '*', // 0x30
	NO, ' ', NO, NO, NO, NO, NO, NO,
	NO, NO, NO, NO, NO, NO, NO, '7', // 0x40
	'8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', NO, NO, NO, NO, // 0x50
	[0xC7] = KEY_HOME,
	[0x9C] = '\n' /*KP_Enter*/,
	[0xB5] = '/' /*KP_Div*/,
	[0xC8] = KEY_UP,
	[0xC9] = KEY_PGUP,
	[0xCB] = KEY_LF,
	[0xCD] = KEY_RT,
	[0xCF] = KEY_END,
	[0xD0] = KEY_DN,
	[0xD1] = KEY_PGDN,
	[0xD2] = KEY_INS,
	[0xD3] = KEY_DEL
};

static uint8_t shiftmap[256] = {
	NO, 033, '!', '@', '#', '$', '%', '^', // 0x00
	'&', '*', '(', ')', '_', '+', '\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', // 0x10
	'O', 'P', '{', '}', '\n', NO, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', // 0x20
	'"', '~', NO, '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', NO, '*', // 0x30
	NO, ' ', NO, NO, NO, NO, NO, NO,
	NO, NO, NO, NO, NO, NO, NO, '7', // 0x40
	'8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', NO, NO, NO, NO, // 0x50
	[0xC7] = KEY_HOME,
	[0x9C] = '\n' /*KP_Enter*/,
	[0xB5] = '/' /*KP_Div*/,
	[0xC8] = KEY_UP,
	[0xC9] = KEY_PGUP,
	[0xCB] = KEY_LF,
	[0xCD] = KEY_RT,
	[0xCF] = KEY_END,
	[0xD0] = KEY_DN,
	[0xD1] = KEY_PGDN,
	[0xD2] = KEY_INS,
	[0xD3] = KEY_DEL
};

#define C(x) (x - '@')

static uint8_t ctlmap[256] = {
	NO, NO, NO, NO, NO, NO, NO, NO,
	NO, NO, NO, NO, NO, NO, NO, NO,
	C('Q'), C('W'), C('E'), C('R'), C('T'), C('Y'), C('U'), C('I'),
	C('O'), C('P'), NO, NO, '\r', NO, C('A'), C('S'),
	C('D'), C('F'), C('G'), C('H'), C('J'), C('K'), C('L'), NO,
	NO, NO, NO, C('\\'), C('Z'), C('X'), C('C'), C('V'),
	C('B'), C('N'), C('M'), NO, NO, C('/'), NO, NO,
	[0x97] = KEY_HOME,
	[0xB5] = C('/'),
	[0xC8] = KEY_UP,
	[0xC9] = KEY_PGUP,
	[0xCB] = KEY_LF,
	[0xCD] = KEY_RT,
	[0xCF] = KEY_END,
	[0xD0] = KEY_DN,
	[0xD1] = KEY_PGDN,
	[0xD2] = KEY_INS,
	[0xD3] = KEY_DEL
};

static uint8_t *charcode[4] = {
	normalmap,
	shiftmap,
	ctlmap,
	ctlmap
};

/**
 * get character from keyboard input
 * 0: not a character, -1: no data
 */
int keyboard_device_getc(void)
{
	int c, v;
	uint8_t data;
	static uint32_t shift;

	if ((inb(KBSTATP) & KBS_DIB) == 0)
		return -1;

	data = inb(KBDATAP);

	/* E0 escape character */
	if (data == 0xE0) {
		shift |= E0ESC;
		return 0;
	}

	/* key released */
	if (data & 0x80) {
		data = (shift & E0ESC ? data : data & 0x7F);
		shift &= ~(shiftcode[data] | E0ESC);
		return 0;
	}

	/* last character was an E0 escape, or with 0x80 */
	if (shift & E0ESC) {
		data |= 0x80;
		shift &= ~E0ESC;
	}

	shift |= shiftcode[data];
	shift ^= togglecode[data];

	v = shift & (CTL | SHIFT);
	c = charcode[v][data];

	if (shift & CAPSLOCK) {
		if ('a' <= c && c <= 'z')
			c += 'A' - 'a';
		else if ('A' <= c && c <= 'Z')
			c += 'a' - 'A';
	}

	/* process special keys, Ctrl+Alt+Del: reboot */
	if (!(~shift & (CTL | ALT)) && c == KEY_DEL)
		reboot();

	return c;
}

static void keyboard_read_all_buffer(void)
{
	int c;

	while ((c = keyboard_device_getc()) != -1) {
		if (c != 0)
			printk(ch(c));
	}
}

static void keyboard_irq_handler(void)
{
	keyboard_read_all_buffer();
}

void keyboard_init(void)
{
	keyboard_read_all_buffer();

	request_irq(IRQ_KEYBD, keyboard_irq_handler);

	/* enable keyboard interrupt */
	pic_enable(PIC_KEYBD);
}

