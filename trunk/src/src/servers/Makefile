# -----------------------------------------------------------------------------
# Copyright (C) 2006 Oleg Fedorov
# -----------------------------------------------------------------------------

include ../../Makefile.conf

all:
	@echo ---------------------------------\> Building app1
	@make -s -C app1
	@echo ---------------------------------\> Building tty
	@make -s -C tty
	@echo ---------------------------------\> Building vesa
	@make -s -C vesa

clean:
	@make -s -C app1 clean
	@make -s -C fs clean
	@make -s -C tty clean
	@make -s -C vesa clean

# -----------------------------------------------------------------------------