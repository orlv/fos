#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(LIB)

OUTPUT		=	libfos.a

OBJECTS		=	message.o	\
			kill.o		\
			exec.o		\
			kmemmap.o	\
			kmalloc.o	\
			kfree.o		\
			thcreate.o	\
			intattach.o	\
			intdetach.o	\
			resmgrattach.o	\
			dmesg.o		\
			uptime.o	\
			alarm.o		\
			alarm2.o

all:	$(OBJECTS)
	$(AR) rc $(OUTPUT) $(OBJECTS)
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
