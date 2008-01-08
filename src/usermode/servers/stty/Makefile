#----------------------------------------------------------------------------#
#  Copyright (C) 2007 Oleg Fedorov
#  Serial I/O (c) 2007 Gridassov Sergey
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk
OUTPUT		=	stty

OBJECTS		=	main.o		\
			tty.o

LDLIBS		=	-\( -lcpp -lc -lfos -\)

TARGETDIR	=	$(INSTALLDIR)

include $(SYSTEM_SRC)/mk/trg_bin_std.mk

