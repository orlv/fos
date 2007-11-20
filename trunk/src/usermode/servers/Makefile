# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# Modified by:
#	Michael Zhilin 23-08-2007
#       Grindassov Sergey 
#	 17-10-2007 Portable GUI System added
#        18-10-2007 New i8042 server added
#       
# -----------------------------------------------------------------------------

include config.mk

SERVERS = namer fbtty pgs tty speaker hw i8042 stty
#SERVERS += floppy

all: 
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Building $(server) ; $(GREY)\
		$(MAKE) -s -C $(server) || exit 1; \
	)

install:
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Install $(server) ; $(GREY)\
		$(MAKE) -s -C $(server) install || exit 1;\
	)

uninstall:
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Uninstall $(server) ; $(GREY)\
		$(MAKE) -s -C $(server) uninstall || exit 1;\
	)

clean:
	@$(foreach server,$(SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Cleaning $(server) ; $(GREY)\
		$(MAKE) -s -C $(server) clean || exit 1;\
	)

# -----------------------------------------------------------------------------
