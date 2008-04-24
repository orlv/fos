#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libfontconfig.a

OBJECTS		=	fcatomic.o	fcblanks.o	fccache.o	\
			fccfg.o		fccharset.o	fcdbg.o		\
			fcdefault.o	fcdir.o		fcfreetype.o	\
			fcfs.o		fcinit.o	fclang.o	\
			fclist.o	fcmatch.o	fcmatrix.o	\
			fcname.o	fcpat.o		fcserialize.o	\
			fcstr.o		fcxml.o		ftglue.o

CFLAGS		+=	-I. -DHAVE_CONFIG_H -DFONTCONFIG_PATH=\"/etc\" -DFC_CACHEDIR=\"/tmp/fontconfig-cache\"


include $(SYSTEM_SRC)/mk/trg_lib.mk

