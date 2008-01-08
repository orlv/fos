#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Gridassov Sergey
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	i8042

OBJECTS		=	main.o i8042.o keyboard.o mouse.o

LDLIBS		=	-\( -lc -lcpp -lfos -\)

TARGETDIR	=	$(INITRDDIR)/bin

include $(SYSTEM_SRC)/mk/trg_bin_std.mk

