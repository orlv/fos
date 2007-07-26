#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libc.a
OBJECTS		=	close.o		\
			malloc.o	\
			exit.o		\
			lseek.o		\
			open.o		\
			read.o		\
			write.o		\
			sprintf.o	\
			vsprintf.o	\
			printf.o	\
			time.o		\
			stat.o		\
			fstat.o

all:	$(OBJECTS)
	$(AR) rcs $(OUTPUT) $(OBJECTS)

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
