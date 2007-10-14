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
SYSTEM_ROOT	= /home/oleg/fos/src/..
SYSTEM_SRC	= /home/oleg/fos/src/../src
BINDIR		= /home/oleg/fos/src/../boot/modules
INSTALLDIR	= /home/oleg/fos/src/../boot/modules
INCLUDE		= /home/oleg/fos/src/../include
LIB		= /home/oleg/fos/src/../lib
#DEBUG		= -g
FLAGS		= -Wall -nostdlib -nostdinc -I$(INCLUDE) -fno-stack-protector -O2

ASFLAGS		= $(FLAGS)
CFLAGS		= $(FLAGS) -ffreestanding -fno-leading-underscore
CXXFLAGS	= $(FLAGS) -nostdinc++ -fno-exceptions -fno-use-cxa-atexit -fno-rtti -fno-builtin
LDFLAGS		= -nostdlib -L$(LIB)

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
