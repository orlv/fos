#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	floppy

OBJECTS		=	floppy.o dma.o

LDLIBS		=	-\( -lc -lcpp -lfos -\)

TARGETDIR	=	$(INITRDDIR)/bin

include $(SYSTEM_SRC)/mk/trg_bin_std.mk

