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

include ../config.mk

#DEBUG =-g

ASFLAGS		= -DiKERNEL $(DEBUG) -Wall -nostdlib -nostdinc -I$(INCLUDE)
CFLAGS		= -DiKERNEL $(DEBUG) -Wall -O3 -I$(INCLUDE) -nostdinc -ffreestanding -fno-stack-protector 
CXXFLAGS	= -DiKERNEL $(DEBUG) -Wall -O3 -I$(INCLUDE) -nostdinc -nostdinc++ -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin -fno-stack-protector -mregparm=3
LDFLAGS		= -nostdlib -static

.s.o:
	@$(ECHO) "Compiling $<"
	@$(AS) $(ASFLAGS) -c -o $*.o $<

.S.o:
	@$(ECHO) "Compiling $<"
	@$(AS) $(CFLAGS) -c -o $*.o $<

.c.o:
	@$(ECHO) "Compiling $<"
	@$(CC) $(CFLAGS) -c -o $*.o $<

.cpp.o:
	@$(ECHO) "Compiling $<"
	@$(C++) $(CXXFLAGS) -c -o $*.o $<

#-----------------------------------------------------------------------------#

OBJECTS =	main/boot/setup.o		\
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
		main/memory/pager.o		\
		main/gdt.o			\
		main/idt.o			\
		main/syscall.o  		\
		main/main.o

OBJECTS +=	drivers/vesafb.o		\
		drivers/char/tty/tty.o		\
		drivers/char/timer/timer.o	\
		drivers/char/cmos/cmos.o	\
		drivers/pic.o			\
		drivers/fs/modulefs/modulefs.o

OBJECTS +=	klibc/close.o			\
		klibc/lseek.o			\
		klibc/open.o			\
		klibc/read.o			\
		klibc/write.o			\
		klibc/sprintf.o			\
		klibc/vsprintf.o		\
		klibc/printk.o			\
		klibc/stat.o			\
		klibc/fstat.o			\
		klibc/version.o			\
		klibc/tmutex.o

OBJECTS +=	klibcpp/purestub.o

OBJECTS +=	klibfos/intattach.o		\
		klibfos/resmgrattach.o

#-----------------------------------------------------------------------------#

all:    $(OBJECTS)
	@$(ECHO) Kernel objects OK
	@$(ECHO) Linking kernel image \($(SYSTEM_ROOT)/kernel\)
	@$(LD) $(LDFLAGS) -o $(KERNEL_PATHNAME) $(OBJECTS) -Ttext=0x100000 --oformat=elf32-i386 --entry=_start

strip:
	@strip $(KERNEL_PATHNAME)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(KERNEL_PATHNAME)

#-----------------------------------------------------------------------------#
