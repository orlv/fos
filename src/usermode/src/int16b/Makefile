#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk


INSTALLDIR	=	$(TOPDIR)/modules
OUTPUT		=	int16b
OBJECTS		=	main.o

#-----------------------------------------------------------------------------#

all:    $(OBJECTS)
	@echo \ Linking $(OUTPUT)
	@$(LD) $(LDFLAGS) -o $(OUTPUT) $(OBJECTS) --oformat=binary -Tldscript -Ttext=0x500

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
