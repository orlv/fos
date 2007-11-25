#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Gridassov Sergey
#----------------------------------------------------------------------------#

include ../config.mk
CFLAGS+=--std=c99
OUTPUT		=	romfs

OBJECTS		=	main.o romfs.o

LDLIBS		=	-\( -lc -lcpp -lfos -\)

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
