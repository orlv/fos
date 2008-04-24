#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libexpat.a

OBJECTS		=	xmlparse.o	xmlrole.o	xmltok.o

CFLAGS		+=	-I. -DHAVE_EXPAT_CONFIG_H

include $(SYSTEM_SRC)/mk/trg_lib.mk

