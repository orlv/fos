# -----------------------------------------------------------------------------
# Automatically generated, do not edit!
# Copyright (C) 2007 Oleg Fedorov
# -----------------------------------------------------------------------------
CC = gcc
C++ = gcc
AS = gcc
LD = ld
AR = ar
MAKE = make
ECHO = /bin/echo -e

# output coloring:
GREEN		= /bin/echo -en "\033[0;32;40m" ;
YELLOW		= /bin/echo -en "\033[1;33;40m" ;
RED		= /bin/echo -en "\033[0;38;40m" ;
GREY		= /bin/echo -en "\033[1;37;40m" ;

#----------------------------------------------------------------------------#

SYSTEM_NAME     = FOS
SYSTEM_ROOT	= /home/grindars/oses/fos/src/..
SYSTEM_SRC	= /home/grindars/oses/fos/src/../src
BINDIR		= /home/grindars/oses/fos/src/../boot/modules
INSTALLDIR	= /home/grindars/oses/fos/src/../boot/modules
INCLUDE		= /home/grindars/oses/fos/src/../include
LIB		= /home/grindars/oses/fos/src/../lib
#DEBUG		= -g
FLAGS		= -Wall -nostdlib -nostdinc -I$(INCLUDE) -fno-stack-protector -O3 

ASFLAGS		= $(FLAGS)
CFLAGS		= $(FLAGS) -ffreestanding -fno-leading-underscore
CXXFLAGS	= $(FLAGS) -nostdinc++ -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin
LDFLAGS		= -nostdlib -L$(LIB)

ifeq ($(shell uname -m), x86_64)
CFLAGS+=-m32
ASFLAGS+=-m32
CXXFLAGS+=-m32
LDFLAGS+=-melf_i386
endif

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
# -----------------------------------------------------------------------------
