#----------------------------------------------------------------------------#
#	FOS
#	Copyright (C)2004-2007 Oleg Fedorov
#	Makefile
#----------------------------------------------------------------------------#

#----------------------------------------------------------------------------#
CC = gcc
#CC = tcc
C++ = gcc
#AS = tcc
AS = gcc
#LD = tcc
LD = ld
#----------------------------------------------------------------------------#

include ../../Makefile.conf

ASFLAGS =-Wall -nostdlib -nostdinc -Iinclude
CFLAGS  =-Wall -ffreestanding -nostdlib -nostdinc -fno-stack-protector -Iinclude

CXXFLAGS =-Wall -nostdinc -nostdinc++ -Iinclude -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin -fno-stack-protector -O0

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

Objects =	main/boot/setup.o	\
		main/mtask/tasks.o	\
		main/mtask/scheduler.o	\
		main/exception/exception.o     \
		main/exception/traps.o	\
		main/memory/paging.o	\
		main/memory/kmalloc.o	\
		main/memory/heap.o	\
		main/memory/dt.o	\
		main/pure.o		\
		main/main.o		\
		main/version.o		\
		main/tinterface.o	\
		main/fs/layer.o		\
		main/fs/fs.o		\
		lib/vsprintf.o		\
		lib/stdio.o		\
		lib/list.o		\
		lib/time.o		\
		main/syscall/syscall.o  \
		main/syscall/trapgate.o \
		drivers/char/tty/tty.o		\
		drivers/block/vga/vga.o		\
		drivers/char/timer/timer.o	\
		drivers/char/cmos/cmos.o	\
		drivers/block/floppy/floppy.o	\
		drivers/dma.o		\
		drivers/block/hd/hd.o	\
		drivers/fs/modulefs/modulefs.o   \
		drivers/fs/objectfs/objectfs.o	 \
		drivers/char/keyboard/keyboard.o


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
