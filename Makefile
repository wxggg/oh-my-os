PROJ	:= 3.1
EMPTY	:=
SPACE	:= $(EMPTY) $(EMPTY)
SLASH	:= /

V       := @

# eliminate default suffix rules
.SUFFIXES: .c .S .h

# delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# define compiler and flags

HOSTCC		:= gcc
HOSTCFLAGS	:= -g -Wall -O2

GDB		:= $(GCCPREFIX)gdb

CC		:= $(GCCPREFIX)gcc
CFLAGS	:= -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc $(DEFS)
CFLAGS	+= $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
CTYPE	:= c S

LD      := $(GCCPREFIX)ld
LDFLAGS	:= -m $(shell $(LD) -V | grep elf_i386 2>/dev/null)
LDFLAGS	+= -nostdlib

OBJCOPY := $(GCCPREFIX)objcopy
OBJDUMP := $(GCCPREFIX)objdump

COPY	:= cp
MKDIR   := mkdir -p
MV		:= mv
RM		:= rm -f
AWK		:= awk
SED		:= sed
SH		:= sh
TR		:= tr
TOUCH	:= touch -c

OBJDIR	:= obj
BINDIR	:= bin

ALLOBJS	:=
ALLDEPS	:=
TARGETS :=

include tools/function.mk

listf_cc = $(call listf,$(1),$(CTYPE))

# for cc
add_files_cc = $(call add_files,$(1),$(CC),$(CFLAGS) $(3),$(2),$(4))
create_target_cc = $(call create_target,$(1),$(2),$(3),$(CC),$(CFLAGS))

# for hostcc
add_files_host = $(call add_files,$(1),$(HOSTCC),$(HOSTCFLAGS),$(2),$(3))
create_target_host = $(call create_target,$(1),$(2),$(3),$(HOSTCC),$(HOSTCFLAGS))

cgtype = $(patsubst %.$(2),%.$(3),$(1))
objfile = $(call toobj,$(1))
asmfile = $(call cgtype,$(call toobj,$(1)),o,asm)
outfile = $(call cgtype,$(call toobj,$(1)),o,out)
symfile = $(call cgtype,$(call toobj,$(1)),o,sym)

# include kernel/user

INCLUDE	+= include/

CFLAGS	+= $(addprefix -I,$(INCLUDE))

LIBDIR	+= libs

$(call add_files_cc,$(call listf_cc,$(LIBDIR)),libs,)

# kernel

KINCLUDE	+= 	kernel/

KSRCDIR		+= kernel/init \
				drivers/console

KCFLAGS		+= $(addprefix -I,$(KINCLUDE))

$(call add_files_cc,$(call listf_cc,$(KSRCDIR)),kernel,$(KCFLAGS))

KOBJS	= $(call read_packet,kernel libs)

# create kernel target
kernel = $(call totarget,kernel)

$(kernel): tools/kernel.ld

$(kernel): $(KOBJS)
	@echo + ld $@
	$(V)$(LD) $(LDFLAGS) -T tools/kernel.ld -o $@ $(KOBJS)
	@$(OBJDUMP) -S $@ > $(call asmfile,kernel)
	@$(OBJDUMP) -t $@ | $(SED) '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(call symfile,kernel)

$(call create_target,kernel)

# create bootblock
bootfiles = $(call listf_cc,boot/boot)
$(foreach f,$(bootfiles),$(call cc_compile,$(f),$(CC),$(CFLAGS) -Os -nostdinc))

bootblock = $(call totarget,bootblock)

$(bootblock): $(call toobj,boot/boot/bootsect.S) $(call toobj,$(bootfiles)) | $(call totarget,resize_binary)
	@echo + ld $@
	$(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 $^ -o $(call toobj,bootblock)
	@$(OBJDUMP) -S $(call objfile,bootblock) > $(call asmfile,bootblock)
	@$(OBJCOPY) -S -O binary $(call objfile,bootblock) $(call outfile,bootblock)
	@$(call totarget,resize_binary) -i $(call outfile, bootblock) -o $(bootblock) -b

$(call create_target,bootblock)

setupfiles = $(call listf_cc,boot/setup)
$(foreach f,$(setupfiles),$(call cc_compile,$(f),$(CC),$(CFLAGS) -Os -nostdinc))

setupblock = $(call totarget,setupblock)

$(setupblock): $(call toobj,boot/setup/setup.S) $(call toobj,$(setupfiles)) | $(call totarget,resize_binary)
	@echo + ld $@
	$(V)$(LD) $(LDFLAGS) -N -e setup -Ttext 0x7e00 $^ -o $(call toobj,setupblock)
	@$(OBJDUMP) -S $(call objfile,setupblock) > $(call asmfile,setupblock)
	@$(OBJCOPY) -S -O binary $(call objfile,setupblock) $(call outfile,setupblock)
	@$(call totarget,resize_binary) -i $(call outfile,setupblock) -o $(setupblock) -n 1024

$(call create_target,setupblock)

# create 'resize_binary' tools
$(call add_files_host,tools/resize_binary.c,resize_binary,resize_binary)
$(call create_target_host,resize_binary,resize_binary)

# create oh-my-os.img
IMG	:= $(call totarget,oh-my-os.img)

$(IMG): $(bootblock) $(setupblock) $(kernel)
	$(V)dd if=/dev/zero of=$@ bs=512 count=10000
	$(V)dd if=$(bootblock) of=$@ conv=notrunc
	$(V)dd if=$(setupblock) of=$@ bs=512 seek=1 conv=notrunc
	$(V)dd if=$(kernel) of=$@ bs=512 seek=3 conv=notrunc
$(call create_target,oh-my-os.img)

$(call finish_all)

TARGETS: $(TARGETS)

.DEFAULT_GOAL := TARGETS

.PHONY: clean
clean:
	find . -type f | xargs touch
	-$(RM) -r $(OBJDIR) $(BINDIR)