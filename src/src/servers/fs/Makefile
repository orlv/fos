#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../Makefile.inc

Objects  = main.o \
	tinterface.o \
	parse.o \
	layer.o \
	fs/objectfs/objectfs.o \
	../lib/heap.o \
	../lib/libcpp.o \
	../lib/stdio.o \
	../lib/vsprintf.o \
	../lib/list.o

PROGNAME = fs

#-----------------------------------------------------------------------------#

all:    $(Objects)
	@echo \ Linking $(BINDIR)/$(PROGNAME)
	@$(LD) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME) --oformat=elf32-i386 --entry=_start $(Objects) --Ttext=0x3200000

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(Objects)
	@rm -f $(BINDIR)/$(PROGNAME)

#-----------------------------------------------------------------------------#
