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
DEBUG = -DDEBUG_SERIAL

ASFLAGS		= -DiKERNEL $(DEBUG) -Wall -nostdlib -nostdinc -I$(INCLUDE)
CFLAGS		= -DiKERNEL $(DEBUG) -Wall -O2 -I$(INCLUDE) -nostdinc -ffreestanding -fno-stack-protector --std=gnu99
CXXFLAGS	= -DiKERNEL $(DEBUG) -Wall -O2 -I$(INCLUDE) -nostdinc -nostdinc++ -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin -fno-stack-protector -mregparm=3
LDFLAGS		= -nostdlib -static

ifeq ($(shell uname -m), x86_64)
ASFLAGS+=-m32
CFLAGS+=-m32
CXXFLAGS+=-m32
LDFLAGS+=-melf_i386
endif

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
		main/system.o			\
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
		main/idt.o			\
		main/syscall.o  		\
		main/main.o			\
		main/ipc.o			\
		main/version.o

OBJECTS +=	drivers/vesa/vesafb.o		\
		drivers/tty/tty.o		\
		drivers/timer/timer.o		\
		drivers/pic/pic.o		\
		drivers/modulefs/modulefs.o

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
		klibc/strdup.o

OBJECTS +=	klibcpp/purestub.o

OBJECTS +=	klibfos/intattach.o		\
		klibfos/resmgrattach.o		\
		klibfos/nsi.o

OBJECTS +=	arch/i386/gdt.o		\
		arch/i386/proc.o

#-----------------------------------------------------------------------------#

all:    $(OBJECTS)
	@$(ECHO) Kernel objects OK
	@$(ECHO) Linking kernel image \($(SYSTEM_ROOT)/kernel\)
	@$(LD) $(LDFLAGS) -o fos  $(OBJECTS) -Ttext=0x100000 --oformat=elf32-i386 --entry=_start

install:
	cp fos $(KERNEL_PATHNAME)
#	strip $(KERNEL_PATHNAME)


#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(KERNEL_PATHNAME)
	@rm -f fos

#-----------------------------------------------------------------------------#
