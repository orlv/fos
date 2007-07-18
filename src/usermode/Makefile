# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

CONFIG      = `pwd`/config.mk
MAIN_CONFIG = $(TOPDIR)/src/config.mk
MKFLAGS     = CONFIG=$(CONFIG) TOPDIR=$(TOPDIR) MAIN_CONFIG=$(MAIN_CONFIG)

all:
	@echo ---------------------------------\> Building namer
	@make -s -C namer $(MKFLAGS)
	@echo ---------------------------------\> Building shell
	@make -s -C shell $(MKFLAGS)
#	@echo ---------------------------------\> Building tty
#	@make -s -C tty $(MKFLAGS)
	@echo ---------------------------------\> Building 16-bit bios interrupt service
	@make -s -C int16b $(MKFLAGS)
	@echo ---------------------------------\> Building keyboard
	@make -s -C keyboard $(MKFLAGS)
	@echo ---------------------------------\> Building init
	@make -s -C init $(MKFLAGS)
	@echo ---------------------------------\> Building floppy
	@make -s -C floppy $(MKFLAGS)
	@echo ---------------------------------\> Building fbtty
	@make -s -C fbtty $(MKFLAGS)
#	@echo ---------------------------------\> Building hd
#	@make -s -C hd $(MKFLAGS)

clean:
	@make -s -C namer clean
	@make -s -C shell clean
#	@make -s -C tty clean
	@make -s -C fbtty clean
	@make -s -C int16b clean
	@make -s -C keyboard clean
	@make -s -C init clean
	@make -s -C floppy clean
#	@make -s -C hd clean

# -----------------------------------------------------------------------------