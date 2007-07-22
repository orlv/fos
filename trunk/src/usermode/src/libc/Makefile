#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(TOPDIR)/lib

OUTPUT		=	libc.a

OBJECTS		=	startup.o	\
			fos.o		\
			malloc.o	\
			libcpp.o	\
			sprintf.o	\
			vsprintf.o	\
			printf.o	\
			msg.o		\
			tmutex.o	\


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
