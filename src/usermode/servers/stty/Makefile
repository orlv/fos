#----------------------------------------------------------------------------#
#  Copyright (C) 2007 Oleg Fedorov
#  Serial I/O (c) 2007 Gridassov Sergey
#----------------------------------------------------------------------------#

include ../config.mk
CFLAGS+=--std=c99
MAKEFLAGS=
CC=gcc
OUTPUT		=	stty

OBJECTS		=	main.o		\
			tty.o

LDLIBS		=	-\( -lcpp -lc -lfos -\)

all:	$(OBJECTS)
	@echo \ Linking $(OUTPUT)
	@$(LD) $(LDFLAGS) -o $(OUTPUT) $(LIB)/crt0.o $(OBJECTS) $(LDLIBS) --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE)

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
