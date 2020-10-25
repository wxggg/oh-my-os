.code16
.section ".text"
.global start

start:
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

    movw $0x800, %di
    jmp main

disk_addr_packet:
    .byte   0x10                        # [0] size of packet 16 bytes
    .byte   0x00                        # [1] reserved always 0
    .word   0x01                        # [2] blocks to read
    .word   0x00                        # [6] transfer buffer(16 bit segment)
    .word   0x00                        # [6] transfer buffer(16 bit offset)
    .long   0x01                        # [8] starting LBA
    .long   0x00                        # [12]used for upper part of 48 bit LBAs

read_a_sect_hd:
    lea     disk_addr_packet,   %si
    movb    $0x42,              %ah
    movb    $0x80,              %dl
    int     $0x13
    ret

read_setup:
	lea		disk_addr_packet, %si
	movw	$0x7e00>>4, 6(%si)
	xorw 	%cx, %cx
loop:
	call    read_a_sect_hd
    lea     disk_addr_packet,   %si
    movl    8(%si),             %eax
    addl    $0x01,              %eax
    movl    %eax,               (disk_addr_packet + 8)

    movl    6(%si),             %eax
    addl    $512>>4,            %eax
    movl    %eax,               (disk_addr_packet + 6)

	incw	%cx
	cmpw	$0x02+1, %cx
	jne		loop

	ret

main:
	call read_setup
	jmp 0x7e00

spin:
	jmp spin
