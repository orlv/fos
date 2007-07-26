#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libcpp.a

OBJECTS		=	tmutex.o	\
			libcpp.o

all:	$(OBJECTS)
	$(AR) rc $(OUTPUT) $(OBJECTS)

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
