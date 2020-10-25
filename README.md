# oh-my-os

This is just a simple operating system for learning.

## compile

To compile oh-my-os, basic compile tools need be installed, such as gcc and
make. The whole thing can be compiled with `make -j4` .

## run

The oh-my-os is only supported on linux platform, and need qemu-system-i386 to
be available. You can just run the `./run.sh` shell.

## debug

You can debug with gdb remote, just run the `debug.sh` shell.

## features

Only the following features is supported now:

* real mode to protect mode.
* set vbe display mode, for now show nothing.
* print info to stdio through serial port

And many more things on the todo list:

* better console input/output
* graphics
* interrupt
* memory management
* process and thread
* ...
