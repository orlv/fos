#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	net

OBJECTS		=	main.o	rtl8139.o	ethernet.o	arp.o	\
			ip.o	icmp.o

LDLIBS		=	-\( -lc -lcpp -lfos -lpci -\)

TARGETDIR	=	$(INITRDDIR)/bin

include $(SYSTEM_SRC)/mk/trg_bin_std.mk

