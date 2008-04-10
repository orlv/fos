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

ASFLAGS		= -D__KERNEL__ $(DEBUG) -Wall -nostdlib -nostdinc -I$(INCLUDE)
CFLAGS		= -D__KERNEL__ $(DEBUG) -Wall -O2 -I$(INCLUDE) -nostdinc -ffreestanding -fno-stack-protector --std=gnu99
CXXFLAGS	= -D__KERNEL__ $(DEBUG) -Wall -O2 -I$(INCLUDE) -nostdinc -nostdinc++ -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin -fno-stack-protector -mregparm=3
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
		main/procman/procman_srv.o	\
		main/procman/scheduler.o	\
		main/procman/process.o		\
		main/procman/thread.o		\
		main/ints.o			\
		main/memory/mmu.o		\
		main/memory/kmalloc.o		\
		main/memory/mm_srv.o		\
		main/memory/vmm.o		\
		main/memory/heap.o		\
		main/memory/pager.o		\
		main/memory/page_pool.o		\
		main/idt.o			\
		main/syscall.o  		\
		main/main.o			\
		main/ipc.o			\
		main/version.o			\
		main/vectors.o

OBJECTS +=	drivers/vesa/vesafb.o		\
		drivers/tty/tty.o		\
		drivers/i8253/i8253.o		\
		drivers/pic/pic.o		\
		drivers/modulefs/modulefs.o	\
		drivers/apic/apic.o		\
		drivers/apic/realmode_code.o

OBJECTS +=	drivers/interfaces/timer.o

OBJECTS +=	libs/libc/close.o		\
		libs/libc/lseek.o		\
		libs/libc/open.o		\
		libs/libc/read.o		\
		libs/libc/write.o		\
		libs/libc/sprintf.o		\
		libs/libc/vsprintf.o		\
		libs/libc/printk.o		\
		libs/libc/stat.o		\
		libs/libc/fstat.o		\
		libs/libc/strdup.o

OBJECTS +=	libs/libcpp/purestub.o

OBJECTS +=	libs/libfos/intattach.o		\
		libs/libfos/resmgrattach.o	\
		libs/libfos/nsi.o

OBJECTS +=	arch/i386/gdt.o			\
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
