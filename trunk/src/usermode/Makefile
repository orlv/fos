# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

CONFIG      = `pwd`/Makefile.inc
MAIN_CONFIG = $(TOPDIR)/src/Makefile.conf
MKFLAGS     = CONFIG=$(CONFIG) TOPDIR=$(TOPDIR) MAIN_CONFIG=$(MAIN_CONFIG)

all:
	@echo ---------------------------------\> Building app1
	@make -s -C app1 $(MKFLAGS)
	@echo ---------------------------------\> Building tty
	@make -s -C tty $(MKFLAGS)
	@echo ---------------------------------\> Building vesa
	@make -s -C vesa $(MKFLAGS)
	@echo ---------------------------------\> Building keyboard
	@make -s -C keyboard $(MKFLAGS)
	@echo ---------------------------------\> Building init
	@make -s -C init $(MKFLAGS)
	@echo ---------------------------------\> Building hd
	@make -s -C hd $(MKFLAGS)

clean:
	@make -s -C app1 clean
	@make -s -C tty clean
	@make -s -C vesa clean
	@make -s -C keyboard clean
	@make -s -C init clean
	@make -s -C hd clean

# -----------------------------------------------------------------------------