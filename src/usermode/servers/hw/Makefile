# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Michael Zhilin
# -----------------------------------------------------------------------------

include ../config.mk

HW_SERVERS = pci ata

all: 
	@$(foreach server,$(HW_SERVERS),\
		$(ECHO) ---------------------------------\> Building $(server);\
		$(MAKE) -s -C $(server);\
	)

install:
	@$(foreach server,$(HW_SERVERS),\
		$(ECHO) ---------------------------------\> Install $(server);\
		$(MAKE) -s -C $(server) install;\
	)

uninstall:
	@$(foreach server,$(HW_SERVERS),\
		$(ECHO) ---------------------------------\> Uninstall $(server);\
		$(MAKE) -s -C $(server) uninstall;\
	)

clean:
	@$(foreach server,$(HW_SERVERS),\
		$(ECHO) ---------------------------------\> Cleaning $(server);\
		$(MAKE) -s -C $(server) clean;\
	)

# -----------------------------------------------------------------------------
