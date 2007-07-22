#----------------------------------------------------------------------------#
#	Copyright (C) 2006-2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(MAIN_CONFIG)
include $(CONFIG)

OBJECTS  = 	../lib/startup.o \
		main.o \
		../lib/fos.o \
		../lib/heap.o \
		../lib/libcpp.o \
		../lib/vsprintf.o \
		../lib/stdio.o \
		../lib/msg.o

PROGNAME = init

#-----------------------------------------------------------------------------#

all:    $(OBJECTS)
	@echo \ Linking $(BINDIR)/$(PROGNAME)
	@$(LD) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME) --oformat=elf32-i386 --entry=_start $(OBJECTS) --Ttext=$(USER_MEM_BASE)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(BINDIR)/$(PROGNAME)

#-----------------------------------------------------------------------------#
