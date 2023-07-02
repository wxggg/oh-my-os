#include <asm-generic/mmu.h>

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG,        0x8           # kernel code segment selector
.set PROT_MODE_DSEG,        0x10          # kernel data segment selector
.set CR0_PE_ON,             0x1           # protected mode enable flag
.set SMAP,                  0x534d4150
.code16
.section ".text"
.global setup
setup:
	jmp main

set_video_mode:
check_vbe:
	mov $0x4f00, %ax        # VBE
	int $0x10
	cmp $0x004f, %ax        # if VBE exist, ax==0x004f
	jne set_mode_vga13
	movw 0x04(%di), %ax
	cmp $0x0200, %ax	    # version should > 2.0
	jb set_mode_vga13
check_vbe_mode:
	movw $0x118, %cx
	mov $0x4f01, %ax
	int $0x10
	cmp $0x004f, %ax
	jne set_mode_vga13
	movw (%di), %ax
	andw $0x0080, %ax
	jz set_mode_vga13
save_video_mode_info:       # save video info to base address 0x0
	movw $0x118, 0x2        # vmode
	movw 0x12(%di), %ax
	movw %ax, 0x4           # screen width
	movw 0x14(%di), %ax
	movw %ax, 0x6           # screen height
	movb 0x19(%di), %al
	movb %al, 0x8           # bitspixel
	movb 0x1b(%di), %al
	movb %al, 0x9           # mem_model
	movl 0x28(%di), %eax
	movl %eax, 0xc          # vram address
set_mode_vbe:
	movw $0x118, %bx
	addw $0x4000, %bx
	mov $0x4f02, %ax
	int $0x10
	ret
set_mode_vga13:
	mov $0x0013, %ax
	int $0x10
	ret

main:
	call set_video_mode

# Enable A20:
#  For backwards compatibility with the earliest PCs, physical
#  address line 20 is tied low, so that addresses higher than
#  1MB wrap around to zero by default. This code undoes this.
	seta20.1:
	inb $0x64, %al                                  # Wait for not busy
	testb $0x2, %al

	movb $0xd1, %al                                 # 0xd1 -> port 0x64
	outb %al, $0x64

	seta20.2:
	inb $0x64, %al                                  # Wait for not busy
	testb $0x2, %al
	jnz seta20.2

	movb $0xdf, %al                                 # 0xdf -> port 0x60
	outb %al, $0x60

probe_memory:
	movl $0, 0x8000
	xorl %ebx, %ebx
	movw $0x8004, %di
start_probe:
	movl $0xE820, %eax
	movl $20, %ecx
	movl $SMAP, %edx
	int $0x15
	jnc cont
	movw $12345, 0x8000
	jmp finish_probe
cont:
	addw $20, %di
	incl 0x8000
	cmpl $0, %ebx
	jnz start_probe
finish_probe:

# Switch from real to protected mode, using a bootstrap GDT
# and segment translation that makes virtual addresses
# identical to physical addresses, so that the
# effective memory map does not change during the switch.
	cli
	lgdt gdtdesc
	movl %cr0, %eax
	orl $CR0_PE_ON, %eax
	movl %eax, %cr0

# Jump to next instruction, but in 32-bit code segment.
# Switches processor into 32-bit mode.
	ljmp $PROT_MODE_CSEG, $protcseg

.code32                                             # Assemble for 32-bit mode
protcseg:
# Set up the protected-mode data segment registers
	movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
	movw %ax, %ds                                   # -> DS: Data Segment
	movw %ax, %es                                   # -> ES: Extra Segment
	movw %ax, %fs                                   # -> FS
	movw %ax, %gs                                   # -> GS
	movw %ax, %ss                                   # -> SS: Stack Segment

# Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
	movl $0x0, %ebp
	movl $0x7c00, %esp

	call bootmain

.data
# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
gdt:
	SEG_NULLASM                                     # null seg
	SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
	SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
	.word 0x17                                      # sizeof(gdt) - 1
	.long gdt                                       # address gdt
