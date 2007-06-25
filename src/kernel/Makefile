#----------------------------------------------------------------------------#
# FOS
# Copyright (C)2004-2007 Oleg Fedorov
#----------------------------------------------------------------------------#

#----------------------------------------------------------------------------#
CC = gcc
C++ = gcc
AS = gcc
LD = ld
#----------------------------------------------------------------------------#

include ../Makefile.conf

ASFLAGS =-Wall -nostdlib -nostdinc -Iinclude
CFLAGS  =-Wall -ffreestanding -nostdlib -nostdinc -fno-stack-protector -Iinclude

CXXFLAGS =-Wall -nostdinc -nostdinc++ -Iinclude -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin -fno-stack-protector -O0 -mregparm=3

LDFLAGS =-nostdlib -static


.s.o:
	@echo "Compiling $<"
	@$(AS) $(ASFLAGS) -c -o $*.o $<

.S.o:
	@echo "Compiling $<"
	@$(AS) $(CFLAGS) -c -o $*.o $<

.c.o:
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c -o $*.o $<

.cpp.o:
	@echo "Compiling $<"
	@$(C++) $(CXXFLAGS) -c -o $*.o $<


#-----------------------------------------------------------------------------#

Objects =	main/boot/setup.o		\
		main/hal.o			\
		main/procman/procman.o		\
		main/procman/scheduler.o	\
		main/procman/process.o		\
		main/procman/thread.o		\
		main/ints.o			\
		main/memory/mmu.o		\
		main/memory/kmalloc.o		\
		main/memory/heap.o		\
		main/memory/memory.o		\
		main/gdt.o			\
		main/idt.o			\
		main/main.o			\
		main/tinterface.o		\
		main/fs/namer.o			\
		main/fs/parse.o			\
		lib/vsprintf.o			\
		lib/stdio.o			\
		lib/time.o			\
		lib/pure.o			\
		lib/version.o			\
		main/syscall.o  		\
		drivers/char/tty/tty.o		\
		drivers/block/vga/vga.o		\
		drivers/char/timer/timer.o	\
		drivers/char/cmos/cmos.o	\
		drivers/dma.o			\
		drivers/pic.o			\
		drivers/fs/modulefs/modulefs.o

#-----------------------------------------------------------------------------#

all:    $(Objects)
	@echo Kernel objects OK
ifeq ($(LD), tcc)
	@$(LD) $(LDFLAGS) -o $(SYSTEM_ROOT)/kernel $(Objects) -Wl,-Ttext,0x100000 -Wl,--oformat,elf32-i386
else
	@echo Linking kernel image \($(SYSTEM_ROOT)/kernel\)
	@$(LD) $(LDFLAGS) -o $(SYSTEM_ROOT)/kernel $(Objects) -Ttext=0x100000 --oformat=elf32-i386 --entry=_start
endif

strip:
	@strip $(SYSTEM_ROOT)/kernel

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(Objects)
	@rm -f $(SYSTEM_ROOT)/kernel

#-----------------------------------------------------------------------------#
