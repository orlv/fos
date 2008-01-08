#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	int16b
OBJECTS		=	main.o
TARGETDIR	=	$(INSTALLDIR)
LDFLAGS		+=	 --oformat=binary -Tldscript -Ttext=0x500

include $(SYSTEM_SRC)/mk/trg_bin.mk

