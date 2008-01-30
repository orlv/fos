#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	vesa.bin
OBJECTS		=	main.o
TARGETDIR	=	$(INITRDDIR)/lib
LDFLAGS		+=	 --oformat=binary -Tldscript -Ttext=0x500

include $(SYSTEM_SRC)/mk/trg_bin.mk

