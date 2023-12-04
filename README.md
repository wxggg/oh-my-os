# oh-my-os

This is just a simple operating system for learning.

## compile

To compile oh-my-os, basic compile tools need be installed, such as gcc and
make. The whole thing can be compiled with `make -j4` .


## dependency

All linux platform on x86 are supported, the following tools are needed
* qemu-system-i386
* gcc

## run

run with graphic:
```
./run.sh
```

run with nographic
```
./run_nographic.sh
```

```shell
loading...
~/oh-my-os <main*> # ls
sys proc bin
~/oh-my-os <main*> # cd sys
~/oh-my-os <main*> # ls
vma free_vma kmalloc-early kmalloc free_pages gpu_dump
~/oh-my-os <main*> # cat vma
free <0x00001000, 0xc0000000>

~/oh-my-os <main*> # cat kmalloc
max_size:1024
max_order:7
size:8 slabs_full:0 slabs_partial:1 1
size:16 slabs_full:0 slabs_partial:1 5
size:32 slabs_full:0 slabs_partial:1 21
size:256 slabs_full:0 slabs_partial:1 1

~/oh-my-os <main*> # dmesg
[0.0	][debug     ][info ] debug init success
[0.0	][memory    ][info ] BIOS-provided physical RAM map:
[0.0	][memory    ][info ] BIOS-e820: [System RAM 	<0x00000000, 0x0009fbff>]
[0.0	][memory    ][info ] BIOS-e820: [Reserved 	<0x0009fc00, 0x0009ffff>]
[0.0	][memory    ][info ] BIOS-e820: [Reserved 	<0x000f0000, 0x000fffff>]
[0.0	][memory    ][info ] BIOS-e820: [System RAM 	<0x00100000, 0x7ffdffff>]
[0.0	][memory    ][info ] BIOS-e820: [Reserved 	<0x7ffe0000, 0x7fffffff>]
[0.0	][memory    ][info ] BIOS-e820: [Reserved 	<0xfffc0000, 0xffffffff>]
[0.0	][memory    ][info ] physical memory frame number ranges:
[0.0	][memory    ][info ] kernel:	<0x00000000, 0x00002160>
[0.0	][memory    ][info ] linear:	<0x00002160, 0x00038000>
[0.0	][memory    ][info ] highmem:	<0x00038000, 0x0007ffe0>
[0.0	][page      ][info ] add free pages, pfn:<0x00002160, 0x00038000>, linear mem
[0.0	][page      ][debug] start end:<0x00002160, 0x00038000>, split start end:<0x00002400, 0x00038000>
[0.0	][page      ][info ] add free pages, pfn:<0x00038000, 0x0007ffe0>, high mem
[0.0	][page      ][debug] start end:<0x00038000, 0x0007ffe0>, split start end:<0x00038000, 0x0007fc00>
[0.0	][page table][debug] map: <0xc0000000->0x00000000> size:0x38000000
[0.0	][memory    ][info ] memory init success
[0.0	][PIC       ][info ] pic init finished
[0.0	][irq       ][info ] idt init success
[0.0	][irq       ][info ] register handler for irq 32
[0.0	][timer     ][info ] init timer success
[0.0	][irq       ][info ] register handler for irq 33
[0.0	][init      ][info ] kernel init success!
[0.0	][gpu       ][info ] screen:1024x768, pixel bits:24, vram:0xfd000000
[0.0	][page table][debug] map: <0xfd000000->0xfd000000> size:0x00240000
[0.1	][graphic   ][info ] init graphic success
~/oh-my-os <main*> #
```

## arch
* real mode to protect mode.
* print info to stdio through serial port

## memory management

* add buddy allogrithm to allocate/free pages
* kmalloc early: use buddy algorithm to allocate reserved memory
* kmalloc: use slab algorithm to split page and manage small buffer

## interrupt
* i8259 interrupt handler
* support keyboard and timer
* support hardware error detect, for example page fault

## schedule
* support thread create and schedule

## virtual file system
* add virtual file system
* sysfs: support dump kernel info, for example the memory info
* procfs: support dump thread info
* binfs: support virtual program

## graphic
* set vbe display mode, show grid.
