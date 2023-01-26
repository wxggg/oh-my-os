#/bin/sh

make -j4

qemu-system-i386 -S -s bin/oh-my-os.img -serial null  -serial stdio &
sleep 2

gdb -q -x tools/gdbinit\
