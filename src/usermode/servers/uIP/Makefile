#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Gridassov Sergey
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	uip

OBJECTS		=	main.o		rtl8139.o

OBJECTS		+= 	core/timer.o 	core/clock-arch.o	\
			core/psock.o	core/uip-fw.o 		\
			core/uip-neighbor.o	core/uip-split.o	\
			core/uip.o	core/uip_arp.o	\
			core/uiplib.o

OBJECTS		+=	httpd/http-strings.o	httpd/httpd-cgi.o	\
			httpd/httpd-fs.o	httpd/httpd.o

OBJECTS		+=	dhcpc/dhcpc.o

OBJECTS		+=	resolv/resolv.o

LDLIBS		=	-\( -lc -lcpp -lfos -lpci -\)

TARGETDIR	=	$(INITRDDIR)/bin

CFLAGS+=-Icore -Ihttpd -Idhcpc -Iresolv

include $(SYSTEM_SRC)/mk/trg_bin_std.mk

httpd/http-strings.c: httpd/http-strings.in
	./makestrings $< $@

httpd/httpd-fsdata.c: 
	(cd httpd; ./makefsdata)

httpd/httpd-fs.o: httpd/httpd-fsdata.c