#pragma once

/* Application segment type bits */
#define STA_X 0x8 // Executable segment
#define STA_E 0x4 // Expand down (non-executable segments)
#define STA_C 0x4 // Conforming code segment (executable only)
#define STA_W 0x2 // Writeable (non-executable segments)
#define STA_R 0x2 // Readable (executable segments)
#define STA_A 0x1 // Accessed

/* System segment type bits */
#define STS_T16A 0x1 // Available 16-bit TSS
#define STS_LDT  0x2 // Local Descriptor Table
#define STS_T16B 0x3 // Busy 16-bit TSS
#define STS_CG16 0x4 // 16-bit Call Gate
#define STS_TG   0x5 // Task Gate / Coum Transmitions
#define STS_IG16 0x6 // 16-bit Interrupt Gate
#define STS_TG16 0x7 // 16-bit Trap Gate
#define STS_T32A 0x9 // Available 32-bit TSS
#define STS_T32B 0xB // Busy 32-bit TSS
#define STS_CG32 0xC // 32-bit Call Gate
#define STS_IG32 0xE // 32-bit Interrupt Gate
#define STS_TG32 0xF // 32-bit Trap Gate

/* segment */
#define SEG_KTEXT 1 /* kernel text segment */
#define SEG_KDATA 2 /* kernel data segment */
#define SEG_UTEXT 3 /* user text segment */
#define SEG_UDATA 4 /* user data segment */
#define SEG_TSS   5 /* task segment */

/* global descrptor numbers */
#define GD_KTEXT ((SEG_KTEXT) << 3) /* kernel text */
#define GD_KDATA ((SEG_KDATA) << 3) /* kernel data */
#define GD_UTEXT ((SEG_UTEXT) << 3) /* user text */
#define GD_UDATA ((SEG_UDATA) << 3) /* user data */
#define GD_TSS   ((SEG_TSS) << 3)   /* task segment selector */

#define DPL_KERNEL (0)
#define DPL_USER   (3)

#define KERNEL_CS ((GD_KTEXT) | DPL_KERNEL)
#define KERNEL_DS ((GD_KDATA) | DPL_KERNEL)
#define USER_CS   ((GD_UTEXT) | DPL_USER)
#define USER_DS   ((GD_UDATA) | DPL_USER)

/* Normal segment */
#define SEG_NULLASM                                                            \
    .word 0, 0;                                                                \
    .byte 0, 0, 0, 0

#define SEG_ASM(type, base, lim)                                               \
    .word(((lim) >> 12) & 0xffff), ((base)&0xffff);                            \
    .byte(((base) >> 16) & 0xff), (0x90 | (type)),                             \
        (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#define PAGE_OFFSET 12
#define PAGE_SIZE 4096
#define KERNEL_STACK_SIZE ((PAGE_SIZE) * 2)
#define KERNEL_VADDR_SHIFT 0xC0000000

#define vaddr(x) (x + KERNEL_VADDR_SHIFT)
#define paddr(x) (x - KERNEL_VADDR_SHIFT)

#define __pa(x) (x - KERNEL_VADDR_SHIFT)
#define __kva(x) (x + KERNEL_VADDR_SHIFT)
