#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	fc-cache
OBJECTS		=	fc-cache.o
LDLIBS		=	-\( -lc -lcpp -lfos -lfontconfig -lexpat -lfreetype -\)
TARGETDIR	=	$(INITRDDIR)/usr/bin

CFLAGS		+=	-I$(SYSTEM_SRC)/usermode/libs/fontconfig-2.5.91 -DHAVE_CONFIG_H -DFONTCONFIG_PATH=\"/etc\" -DFC_CACHEDIR=\"/tmp/fontconfig-cache\"
include $(SYSTEM_SRC)/mk/trg_bin_std.mk

