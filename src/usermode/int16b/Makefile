#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

OUTPUT		=	int16b
OBJECTS		=	main.o

#-----------------------------------------------------------------------------#

all:    $(OBJECTS)
	@echo \ Linking $(OUTPUT)
	@$(LD) $(LDFLAGS) -o $(OUTPUT) $(OBJECTS) --oformat=binary -Tldscript -Ttext=0x500

install:
	cp $(OUTPUT) $(INSTALLDIR)/

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
