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

## debug

You can debug with gdb remote, just run the `debug.sh` shell.

## arch
* real mode to protect mode.
* print info to stdio through serial port

## memory management

* add buddy allogrithm to allocate/free pages

## graphic
* set vbe display mode, show grid.

## more
* interrupt
* process and thread
* ...
