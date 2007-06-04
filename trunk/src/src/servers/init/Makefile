#----------------------------------------------------------------------------#
#	Copyright (C) 2006-2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../Makefile.inc

Objects  = ../lib/startup.o \
	main.o \
	../lib/fos.o \
	../lib/heap.o \
	../lib/libcpp.o \
	../lib/vsprintf.o \
	../lib/stdio.o


PROGNAME = init

#-----------------------------------------------------------------------------#

all:    $(Objects)
	@echo \ Linking $(BINDIR)/$(PROGNAME)
	@$(LD) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME) --oformat=elf32-i386 --entry=_start $(Objects) --Ttext=0x3200000

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(Objects)
	@rm -f $(BINDIR)/$(PROGNAME)

#-----------------------------------------------------------------------------#
