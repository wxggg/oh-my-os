#include <x86.h>
#include <debug.h>
#include <register.h>
#include <elf.h>

#define ELFHDR   ((struct elfhdr *)0x10000) // scratch space

void ap_bootmain(void)
{
	/* while (1){} */
	// call the entry point from the ELF header
	// note: does not return
	((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();

	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8E00);
}
