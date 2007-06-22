#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../Makefile.inc

Objects  = ../lib/startup.o \
	main.o \
	../lib/fos.o \
	../lib/heap.o \
	../lib/libcpp.o \
	vga.o \
	tty.o \
	../lib/msg.o


PROGNAME = tty

#-----------------------------------------------------------------------------#

all:    $(Objects)
	@echo \ Linking $(BINDIR)/$(PROGNAME)
	@$(LD) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME) --oformat=elf32-i386 --entry=_start $(Objects) --Ttext=$(USER_MEM_BASE)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(Objects)
	@rm -f $(BINDIR)/$(PROGNAME)

#-----------------------------------------------------------------------------#
