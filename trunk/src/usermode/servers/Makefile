# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

include config.mk

all:
	@echo ---------------------------------\> Building namer
	$(MAKE) -s -C namer
	@echo ---------------------------------\> Building keyboard
	$(MAKE) -s -C keyboard
	@echo ---------------------------------\> Building floppy
	$(MAKE) -s -C floppy
	@echo ---------------------------------\> Building fbtty
#	$(MAKE) -s -C tty
	$(MAKE) -s -C fbtty


install:
	$(MAKE) -s -C namer install
#	$(MAKE) -s -C tty install
	$(MAKE) -s -C fbtty install
	$(MAKE) -s -C keyboard install
	$(MAKE) -s -C floppy install

uninstall:
	$(MAKE) -s -C namer uninstall
#	$(MAKE) -s -C tty uninstall
	$(MAKE) -s -C fbtty uninstall
	$(MAKE) -s -C keyboard uninstall
	$(MAKE) -s -C floppy uninstall

clean:
	$(MAKE) -s -C namer clean
#	$(MAKE) -s -C tty clean
	$(MAKE) -s -C fbtty clean
	$(MAKE) -s -C keyboard clean
	$(MAKE) -s -C floppy clean

# -----------------------------------------------------------------------------
