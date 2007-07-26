# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

all:
	@echo ---------------------------------\> Building namer
	make -s -C namer
	@echo ---------------------------------\> Building keyboard
	make -s -C keyboard
	@echo ---------------------------------\> Building floppy
	make -s -C floppy
	@echo ---------------------------------\> Building fbtty
#	make -s -C tty
	make -s -C fbtty


install:
	make -s -C namer install
#	make -s -C tty install
	make -s -C fbtty install
	make -s -C keyboard install
	make -s -C floppy install

uninstall:
	make -s -C namer uninstall
#	make -s -C tty uninstall
	make -s -C fbtty uninstall
	make -s -C keyboard uninstall
	make -s -C floppy uninstall

clean:
	make -s -C namer clean
#	make -s -C tty clean
	make -s -C fbtty clean
	make -s -C keyboard clean
	make -s -C floppy clean

# -----------------------------------------------------------------------------
