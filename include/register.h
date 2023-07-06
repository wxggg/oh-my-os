#pragma once

#include <types.h>

static inline uint32_t read_eflags(void) __attribute__((always_inline));
static inline void write_eflags(uint32_t eflags) __attribute__((always_inline));

static inline void lcr0(uintptr_t cr0) __attribute__((always_inline));
static inline void lcr3(uintptr_t cr3) __attribute__((always_inline));

static inline uintptr_t rcr0(void) __attribute__((always_inline));
static inline uintptr_t rcr1(void) __attribute__((always_inline));
static inline uintptr_t rcr2(void) __attribute__((always_inline));
static inline uintptr_t rcr3(void) __attribute__((always_inline));

static inline uint32_t read_eip(void) __attribute__((always_inline));

static inline uint32_t read_edi(void) __attribute__((always_inline));
static inline uint32_t read_esi(void) __attribute__((always_inline));
static inline uint32_t read_ebp(void) __attribute__((always_inline));
static inline uint32_t read_esp(void) __attribute__((always_inline));
static inline uint32_t read_ebx(void) __attribute__((always_inline));
static inline uint32_t read_edx(void) __attribute__((always_inline));
static inline uint32_t read_ecx(void) __attribute__((always_inline));
static inline uint32_t read_eax(void) __attribute__((always_inline));

static inline uint32_t read_eflags(void)
{
	uint32_t eflags;
	asm volatile("pushfl; popl %0" : "=r"(eflags));
	return eflags;
}

static inline void write_eflags(uint32_t eflags)
{
	asm volatile("pushl %0; popfl" ::"r"(eflags));
}

static inline void lcr0(uintptr_t cr0)
{
	asm volatile("mov %0, %%cr0" ::"r"(cr0) : "memory");
}

static inline void lcr3(uintptr_t cr3)
{
	asm volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

static inline uintptr_t rcr0(void)
{
	uintptr_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0)::"memory");
	return cr0;
}

static inline uintptr_t rcr1(void)
{
	uintptr_t cr1;
	asm volatile("mov %%cr1, %0" : "=r"(cr1)::"memory");
	return cr1;
}

static inline uintptr_t rcr2(void)
{
	uintptr_t cr2;
	asm volatile("mov %%cr2, %0" : "=r"(cr2)::"memory");
	return cr2;
}

static inline uintptr_t rcr3(void)
{
	uintptr_t cr3;
	asm volatile("mov %%cr3, %0" : "=r"(cr3)::"memory");
	return cr3;
}

static inline uint32_t read_eip(void)
{
	uint32_t eip;
	asm volatile ("call 1f \n\t1: pop %0" : "=r"(eip));
	return eip;
}

static inline uint32_t read_edi(void)
{
	uint32_t edi;
	asm volatile("movl %%edi, %0" : "=r" (edi));
	return edi;
}

static inline uint32_t read_esi(void)
{
	uint32_t esi;
	asm volatile("movl %%esi, %0" : "=r" (esi));
	return esi;
}

static inline uint32_t read_ebp(void)
{
	uint32_t ebp;
	asm volatile("movl %%ebp, %0" : "=r"(ebp));
	return ebp;
}

static inline uint32_t read_esp(void)
{
	uint32_t esp;
	asm volatile("movl %%esp, %0" : "=r"(esp));
	return esp;
}

static inline uint32_t read_ebx(void)
{
	uint32_t ebx;
	asm volatile("movl %%ebx, %0" : "=r" (ebx));
	return ebx;
}

static inline uint32_t read_edx(void)
{
	uint32_t edx;
	asm volatile("movl %%edx, %0" : "=r" (edx));
	return edx;
}

static inline uint32_t read_ecx(void)
{
	uint32_t ecx;
	asm volatile("movl %%ecx, %0" : "=r" (ecx));
	return ecx;
}

static inline uint32_t read_eax(void)
{
	uint32_t eax;
	asm volatile("movl %%eax, %0" : "=r" (eax));
	return eax;
}
