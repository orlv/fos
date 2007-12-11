#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

OUTPUT		=	shell

OBJECTS		=	main.o

LDLIBS		=	-\( -lc -lcpp -lfos -\)

all:	$(OBJECTS)
	@echo \ Linking $(OUTPUT)
	@$(LD) $(LDFLAGS) -o $(OUTPUT) $(LIB)/crt0.o $(OBJECTS) $(LDLIBS) --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE)

install:
	cp $(OUTPUT) $(INITRDDIR)/bin/$(OUTPUT)

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INITRDDIR)/bin/$(OUTPUT)
