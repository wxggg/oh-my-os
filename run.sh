#!/bin/sh

make -j4

qemu-system-i386 -parallel none -hda bin/oh-my-os.img -serial stdio