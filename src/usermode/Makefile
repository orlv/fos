# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

include config.mk

all:
	@$(ECHO) ---------------------------------\> Building crt0
	$(MAKE) -s -C crt0 all install
	@$(ECHO) ---------------------------------\> Building libc
	$(MAKE) -s -C libc all install
	@$(ECHO) ---------------------------------\> Building libcpp
	$(MAKE) -s -C libcpp all install
	@$(ECHO) ---------------------------------\> Building libfos
	$(MAKE) -s -C libfos all install
	@$(ECHO) ---------------------------------\> Building init
	$(MAKE) -s -C init all install
	@$(ECHO) ---------------------------------\> Building shell
	$(MAKE) -s -C shell all install
	@$(ECHO) ---------------------------------\> Building test
	$(MAKE) -s -C test all install
	@$(ECHO) ---------------------------------\> Building int16b
	$(MAKE) -s -C int16b all install
	@$(ECHO) ---------------------------------\> Building servers
	$(MAKE) -s -C servers all install

install:
	$(MAKE) -s -C crt0 install
	$(MAKE) -s -C libc install
	$(MAKE) -s -C libcpp install
	$(MAKE) -s -C libfos install
	$(MAKE) -s -C init install
	$(MAKE) -s -C shell install
	$(MAKE) -s -C test install
	$(MAKE) -s -C int16b install
	$(MAKE) -s -C servers install

uninstall:
	$(MAKE) -s -C crt0 uninstall
	$(MAKE) -s -C libc uninstall
	$(MAKE) -s -C libcpp uninstall
	$(MAKE) -s -C libfos uninstall
	$(MAKE) -s -C init uninstall
	$(MAKE) -s -C shell uninstall
	$(MAKE) -s -C test uninstall
	$(MAKE) -s -C int16b uninstall
	$(MAKE) -s -C servers uninstall

clean:
	$(MAKE) -s -C crt0 clean
	$(MAKE) -s -C libc clean
	$(MAKE) -s -C libcpp clean
	$(MAKE) -s -C libfos clean
	$(MAKE) -s -C init clean
	$(MAKE) -s -C shell clean
	$(MAKE) -s -C test clean
	$(MAKE) -s -C int16b clean
	$(MAKE) -s -C servers clean

# -----------------------------------------------------------------------------
