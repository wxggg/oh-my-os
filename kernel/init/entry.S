#include <asm-generic/mmu.h>

.text
.global kern_entry
kern_entry:
	lgdt (paddr(__gdtesc))
	movl $KERNEL_DS, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	ljmp $KERNEL_CS, $relocated

relocated:
	# set up ebp, esp
	movl $0x0, %ebp
	movl $bootstacktop, %esp

	call kern_init

spin:
	jmp spin

.data
.align PAGE_SIZE
	.globl bootstack
bootstack:
	.space KERNEL_STACK_SIZE
	.global bootstacktop
bootstacktop:

.align 4
__gdt:
	SEG_NULLASM
	SEG_ASM(STA_X | STA_R, - KERNEL_VADDR_SHIFT, 0xFFFFFFFF)
	SEG_ASM(STA_W, - KERNEL_VADDR_SHIFT, 0xFFFFFFFF)
__gdtesc:
	.word 0x17  				# sizeof(gdt) - 1
	.long (paddr(__gdt))		# address gdt