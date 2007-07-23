#----------------------------------------------------------------------------#
#	Copyright (C) 2006-2007 Oleg Fedorov
#----------------------------------------------------------------------------#
CC = gcc
C++ = gcc
AS = gcc
LD = ld
AR = ar
#----------------------------------------------------------------------------#

PWD=`pwd`
TOPDIR=../..

FLAGS=-Wall -nostdlib -nostdinc -I$(TOPDIR)/include -fno-stack-protector -O3

ASFLAGS =$(FLAGS)
CFLAGS  =$(FLAGS) -ffreestanding -fno-leading-underscore
CXXFLAGS=$(FLAGS) -nostdinc++ -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin
LDFLAGS =-nostdlib -L../../lib

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

# Базовый адрес пользовательских программ
USER_MEM_BASE = 0x8000000
