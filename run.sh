#!/bin/sh

make -j4

qemu-system-i386 -m 2G -parallel none -hda bin/oh-my-os.img -serial stdio
