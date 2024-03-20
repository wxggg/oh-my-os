#include <asm-generic/mmu.h>
#include <asm-generic/cpu.h>

.text
.global kern_entry
kern_entry:
	lgdt (__gdtesc - KERNEL_VIRT_BASE)
	movl $KERNEL_DS, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	ljmp $KERNEL_CS, $relocated

relocated:
	# set up ebp, esp
	movl (__entry_cnt), %ecx
	movl $KERNEL_STACK_SIZE, %eax
	mull %ecx
	addl $bootstack, %eax

	movl $0x0, %ebp
	addl $KERNEL_STACK_SIZE, %eax
	movl %eax, %esp

	incl (__entry_cnt)

	cmp $1, (__entry_cnt)
	jne secondary

	call start_kernel
	jmp spin

secondary:
	call start_secondary
	jmp spin

spin:
	jmp spin

.data
.align PAGE_SIZE
.globl bootstack
bootstack:
	.space KERNEL_STACK_SIZE
	.space KERNEL_STACK_SIZE
	.space KERNEL_STACK_SIZE
	.space KERNEL_STACK_SIZE
	.space KERNEL_STACK_SIZE
	.space KERNEL_STACK_SIZE
	.space KERNEL_STACK_SIZE
	.space KERNEL_STACK_SIZE
.global bootstacktop
bootstacktop:

.align 4

__entry_cnt:
	.word 0

__gdt:
	SEG_NULLASM
	SEG_ASM(STA_X | STA_R, - KERNEL_VIRT_BASE, 0xFFFFFFFF)
	SEG_ASM(STA_W, - KERNEL_VIRT_BASE, 0xFFFFFFFF)
__gdtesc:
	.word 0x17  				# sizeof(gdt) - 1
	.long (__gdt - KERNEL_VIRT_BASE)	# address gdt

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

.globl run_entrys
run_entrys:
	movl 4(%esp), %esp
	jmp __trapret

.text
.global thread_entry
thread_entry:
	pushl %edx
	call *%ebx

	pushl %eax
	call thread_exit

halt:
	call schedule
	hlt
	jmp halt

stall:
	jmp stall

.text
.globl context_switch
context_switch:                      # context_switch(from, to)
	# save from's registers
	movl 4(%esp), %eax          # eax points to from
	popl 0(%eax)                # save eip !popl
	movl %esp, 4(%eax)
	movl %ebx, 8(%eax)
	movl %ecx, 12(%eax)
	movl %edx, 16(%eax)
	movl %esi, 20(%eax)
	movl %edi, 24(%eax)
	movl %ebp, 28(%eax)

	# restore to's registers
	movl 4(%esp), %eax          # not 8(%esp): popped return address already
	# eax now points to to
	movl 28(%eax), %ebp
	movl 24(%eax), %edi
	movl 20(%eax), %esi
	movl 16(%eax), %edx
	movl 12(%eax), %ecx
	movl 8(%eax), %ebx
	movl 4(%eax), %esp

	pushl 0(%eax)               # push eip

	ret

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
	pushl $8
	jmp __alltraps
.global vector9
vector9:
	pushl $9
	jmp __alltraps
.global vector10
vector10:
	pushl $10
	jmp __alltraps
.global vector11
vector11:
	pushl $11
	jmp __alltraps
.global vector12
vector12:
	pushl $12
	jmp __alltraps
.global vector13
vector13:
	pushl $13
	jmp __alltraps
.global vector14
vector14:
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
.global vector37
vector37:
	pushl $0
	pushl $37
	jmp __alltraps
.global vector38
vector38:
	pushl $0
	pushl $38
	jmp __alltraps
.global vector39
vector39:
	pushl $0
	pushl $39
	jmp __alltraps
.global vector40
vector40:
	pushl $0
	pushl $40
	jmp __alltraps
.global vector41
vector41:
	pushl $0
	pushl $41
	jmp __alltraps
.global vector42
vector42:
	pushl $0
	pushl $42
	jmp __alltraps
.global vector43
vector43:
	pushl $0
	pushl $43
	jmp __alltraps
.global vector44
vector44:
	pushl $0
	pushl $44
	jmp __alltraps
.global vector45
vector45:
	pushl $0
	pushl $45
	jmp __alltraps
.global vector46
vector46:
	pushl $0
	pushl $46
	jmp __alltraps
.global vector47
vector47:
	pushl $0
	pushl $47
	jmp __alltraps
.global vector48
vector48:
	pushl $0
	pushl $48
	jmp __alltraps
.global vector49
vector49:
	pushl $0
	pushl $49
	jmp __alltraps
.global vector50
vector50:
	pushl $0
	pushl $50
	jmp __alltraps
.global vector51
vector51:
	pushl $0
	pushl $51
	jmp __alltraps
.global vector52
vector52:
	pushl $0
	pushl $52
	jmp __alltraps
.global vector53
vector53:
	pushl $0
	pushl $53
	jmp __alltraps
.global vector54
vector54:
	pushl $0
	pushl $54
	jmp __alltraps
.global vector55
vector55:
	pushl $0
	pushl $55
	jmp __alltraps
.global vector56
vector56:
	pushl $0
	pushl $56
	jmp __alltraps
.global vector57
vector57:
	pushl $0
	pushl $57
	jmp __alltraps
.global vector58
vector58:
	pushl $0
	pushl $58
	jmp __alltraps
.global vector59
vector59:
	pushl $0
	pushl $59
	jmp __alltraps
.global vector60
vector60:
	pushl $0
	pushl $60
	jmp __alltraps
.global vector61
vector61:
	pushl $0
	pushl $61
	jmp __alltraps
.global vector62
vector62:
	pushl $0
	pushl $62
	jmp __alltraps
.global vector63
vector63:
	pushl $0
	pushl $63
	jmp __alltraps
.global vector64
vector64:
	pushl $0
	pushl $64
	jmp __alltraps
.global vector65
vector65:
	pushl $0
	pushl $65
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
	.long vector37
	.long vector38
	.long vector39
	.long vector40
	.long vector41
	.long vector42
	.long vector43
	.long vector44
	.long vector45
	.long vector46
	.long vector47
	.long vector48
	.long vector49
	.long vector50
	.long vector51
	.long vector52
	.long vector53
	.long vector54
	.long vector55
	.long vector56
	.long vector57
	.long vector58
	.long vector59
	.long vector60
	.long vector61
	.long vector62
	.long vector63
	.long vector64
	.long vector65
