# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# Modified by:
#	Michael Zhilin 23-08-2007
# -----------------------------------------------------------------------------

include config.mk

SERVERS = namer keyboard floppy fbtty
#SERVERS += tty

all: 
	@$(foreach server,$(SERVERS),\
		$(ECHO) ---------------------------------\> Building $(server);\
		$(MAKE) -s -C $(server);\
	)

install:
	@$(foreach server,$(SERVERS),\
		$(ECHO) ---------------------------------\> Install $(server);\
		$(MAKE) -s -C $(server) install;\
	)

uninstall:
	@$(foreach server,$(SERVERS),\
		$(ECHO) ---------------------------------\> Uninstall $(server);\
		$(MAKE) -s -C $(server) uninstall;\
	)

clean:
	@$(foreach server,$(SERVERS),\
		$(ECHO) ---------------------------------\> Cleaning $(server);\
		$(MAKE) -s -C $(server) clean;\
	)

# -----------------------------------------------------------------------------
