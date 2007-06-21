#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
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

FLAGS=-Wall -nostdlib -nostdinc -I../include -I. -L../lib -fno-stack-protector -O3

ASFLAGS =$(FLAGS)
CFLAGS  =$(FLAGS) -ffreestanding -fno-leading-underscore
CXXFLAGS=$(FLAGS) -nostdinc++ -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin
LDFLAGS =-nostdlib -L=$(BINDIR)

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