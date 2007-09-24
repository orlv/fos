# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# Modified by:
#	Michael Zhilin 23-08-2007
# -----------------------------------------------------------------------------

include config.mk

SERVERS = namer keyboard fbtty tty speaker hw
#SERVERS += floppy

all: 
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Building $(server) ; $(GREY)\
		$(MAKE) -s -C $(server);\
	)

install:
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Install $(server) ; $(GREY)\
		$(MAKE) -s -C $(server) install;\
	)

uninstall:
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Uninstall $(server) ; $(GREY)\
		$(MAKE) -s -C $(server) uninstall;\
	)

clean:
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Cleaning $(server) ; $(GREY)\
		$(MAKE) -s -C $(server) clean;\
	)

# -----------------------------------------------------------------------------
