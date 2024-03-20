#!/bin/sh

make clean

make -j4

qemu-system-i386 \
	-d int,pcall,mmu,cpu_reset,guest_errors \
	-D ./error.log \
	-nographic -smp 4 -m 2G -parallel none -hda bin/oh-my-os.img -serial mon:stdio
