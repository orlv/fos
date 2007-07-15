#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(MAIN_CONFIG)
include $(CONFIG)

OBJECTS  = main.o
PROGNAME = int16b

#-----------------------------------------------------------------------------#

all:    $(OBJECTS)
	@echo \ Linking $(BINDIR)/$(PROGNAME)
	@$(LD) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME) $(OBJECTS) --oformat=binary -Tldscript -Ttext=0x500
#	$(LD) $(LDFLAGS) -shared -o $(BINDIR)/$(PROGNAME) $(OBJECTS) --oformat=elf32-i386

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(BINDIR)/$(PROGNAME)

#-----------------------------------------------------------------------------#
