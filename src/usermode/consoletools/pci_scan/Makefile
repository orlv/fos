#----------------------------------------------------------------------------#
#	Copyright (C) 2006 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

OUTPUT		=	pci_scan

OBJECTS		=	main.o devs.o pci.o config.o

LDLIBS		=	-\( -lc -lcpp -lfos -lpci -\)

TARGETDIR	=	$(INITRDDIR)/bin

include $(SYSTEM_SRC)/mk/trg_bin_std.mk

