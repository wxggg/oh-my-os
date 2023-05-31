#include <asm-generic/mmu.h>
#include <asm-generic/cpu.h>

.text
.global kern_entry
kern_entry:
	lgdt (__gdtesc - KERNEL_VADDR_SHIFT)
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
	.long (__gdt - KERNEL_VADDR_SHIFT)	# address gdt

.text
.globl __alltraps
__alltraps:
	pushl %ds
	pushl %es
	pushal

	movl $GD_KDATA, %eax
	movw %ax, %ds
	movw %ax, %es

	pushl %esp

	call irq_handler

	popl %esp

.globl __trapret
__trapret:
	popal
	popl %es
	popl %ds

# get rid of the trap number and error code
	addl $0x8, %esp
	iret

.text
.global vector0
vector0:
	pushl $0
	pushl $0
	jmp __alltraps
.global vector1
vector1:
	pushl $0
	pushl $1
	jmp __alltraps
.global vector2
vector2:
	pushl $0
	pushl $2
	jmp __alltraps
.global vector3
vector3:
	pushl $0
	pushl $3
	jmp __alltraps
.global vector4
vector4:
	pushl $0
	pushl $4
	jmp __alltraps
.global vector5
vector5:
	pushl $0
	pushl $5
	jmp __alltraps
.global vector6
vector6:
	pushl $0
	pushl $6
	jmp __alltraps
.global vector7
vector7:
	pushl $0
	pushl $7
	jmp __alltraps
.global vector8
vector8:
	pushl $0
	pushl $8
	jmp __alltraps
.global vector9
vector9:
	pushl $0
	pushl $9
	jmp __alltraps
.global vector10
vector10:
	pushl $0
	pushl $10
	jmp __alltraps
.global vector11
vector11:
	pushl $0
	pushl $11
	jmp __alltraps
.global vector12
vector12:
	pushl $0
	pushl $12
	jmp __alltraps
.global vector13
vector13:
	pushl $0
	pushl $13
	jmp __alltraps
.global vector14
vector14:
	pushl $0
	pushl $14
	jmp __alltraps
.global vector15
vector15:
	pushl $0
	pushl $15
	jmp __alltraps
.global vector16
vector16:
	pushl $0
	pushl $16
	jmp __alltraps
.global vector17
vector17:
	pushl $0
	pushl $17
	jmp __alltraps
.global vector18
vector18:
	pushl $0
	pushl $18
	jmp __alltraps
.global vector19
vector19:
	pushl $0
	pushl $19
	jmp __alltraps
.global vector20
vector20:
	pushl $0
	pushl $20
	jmp __alltraps
.global vector21
vector21:
	pushl $0
	pushl $21
	jmp __alltraps
.global vector22
vector22:
	pushl $0
	pushl $22
	jmp __alltraps
.global vector23
vector23:
	pushl $0
	pushl $23
	jmp __alltraps
.global vector24
vector24:
	pushl $0
	pushl $24
	jmp __alltraps
.global vector25
vector25:
	pushl $0
	pushl $25
	jmp __alltraps
.global vector26
vector26:
	pushl $0
	pushl $26
	jmp __alltraps
.global vector27
vector27:
	pushl $0
	pushl $27
	jmp __alltraps
.global vector28
vector28:
	pushl $0
	pushl $28
	jmp __alltraps
.global vector29
vector29:
	pushl $0
	pushl $29
	jmp __alltraps
.global vector30
vector30:
	pushl $0
	pushl $30
	jmp __alltraps
.global vector31
vector31:
	pushl $0
	pushl $31
	jmp __alltraps
.global vector32
vector32:
	pushl $0
	pushl $32
	jmp __alltraps
.global vector33
vector33:
	  pushl $0
	  pushl $33
	  jmp __alltraps
.global vector34
vector34:
	  pushl $0
	  pushl $34
	  jmp __alltraps
.global vector35
vector35:
	pushl $0
	pushl $35
	jmp __alltraps
.global vector36
vector36:
	pushl $0
	pushl $36
	jmp __alltraps

.data
.globl __vectors
__vectors:
	.long vector0
	.long vector1
	.long vector2
	.long vector3
	.long vector4
	.long vector5
	.long vector6
	.long vector7
	.long vector8
	.long vector9
	.long vector10
	.long vector11
	.long vector12
	.long vector13
	.long vector14
	.long vector15
	.long vector16
	.long vector17
	.long vector18
	.long vector19
	.long vector20
	.long vector21
	.long vector22
	.long vector23
	.long vector24
	.long vector25
	.long vector26
	.long vector27
	.long vector28
	.long vector29
	.long vector30
	.long vector31
	.long vector32
	.long vector33
	.long vector34
	.long vector35
	.long vector36

