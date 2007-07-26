#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	crt0.o
OBJECTS		=	crt0.o

all:		$(OBJECTS)

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
