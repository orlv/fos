# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

all:
	@echo ---------------------------------\> Building crt0
	@make -s -C crt0 all install
	@echo ---------------------------------\> Building libc
	make -s -C libc all install
	@echo ---------------------------------\> Building libcpp
	make -s -C libcpp all install
	@echo ---------------------------------\> Building libfos
	make -s -C libfos all install
	@echo ---------------------------------\> Building init
	make -s -C init all install
	@echo ---------------------------------\> Building shell
	make -s -C shell all install
	@echo ---------------------------------\> Building test
	make -s -C test all install
	@echo ---------------------------------\> Building int16b
	make -s -C int16b all install
	@echo ---------------------------------\> Building servers
	make -s -C servers all install

install:
	make -s -C crt0 install
	make -s -C libc install
	make -s -C libcpp install
	make -s -C libfos install
	make -s -C init install
	make -s -C shell install
	make -s -C test install
	make -s -C int16b install
	make -s -C servers install

uninstall:
	make -s -C crt0 uninstall
	make -s -C libc uninstall
	make -s -C libcpp uninstall
	make -s -C libfos uninstall
	make -s -C init uninstall
	make -s -C shell uninstall
	make -s -C test uninstall
	make -s -C int16b uninstall
	make -s -C servers uninstall

clean:
	make -s -C crt0 clean
	make -s -C libc clean
	make -s -C libcpp clean
	make -s -C libfos clean
	make -s -C init clean
	make -s -C shell clean
	make -s -C test clean
	make -s -C int16b clean
	make -s -C servers clean

# -----------------------------------------------------------------------------
