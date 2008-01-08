#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	tty

OBJECTS		=	main.o		\
			vga.o		\
			tty.o

LDLIBS		=	-\( -lcpp -lc -lfos -\)
TARGETDIR	=	$(INSTALLDIR)

include $(SYSTEM_SRC)/mk/trg_bin_std.mk

