#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../Makefile.inc

Objects  = main.o
PROGNAME = vesa

#-----------------------------------------------------------------------------#

all:    $(Objects)
	@echo \ Linking $(BINDIR)/$(PROGNAME)
	@$(LD) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME) $(Objects) --oformat=binary -Tldscript -Ttext=0x1f00
#	$(LD) $(LDFLAGS) -shared -o $(BINDIR)/$(PROGNAME) $(Objects) --oformat=elf32-i386

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(Objects)
	@rm -f $(BINDIR)/$(PROGNAME)

#-----------------------------------------------------------------------------#
