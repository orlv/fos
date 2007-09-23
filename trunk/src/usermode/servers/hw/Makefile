# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Michael Zhilin
# -----------------------------------------------------------------------------

include ../config.mk

HW_SERVERS = pci ata

all: 
	@$(foreach server,$(HW_SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Building $(server); $(GREY) \
		$(MAKE) -s -C $(server);\
	)

install:
	@$(foreach server,$(HW_SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Install $(server); $(GREY) \
		$(MAKE) -s -C $(server) install;\
	)

uninstall:
	@$(foreach server,$(HW_SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Uninstall $(server); $(GREY) \
		$(MAKE) -s -C $(server) uninstall;\
	)

clean:
	@$(foreach server,$(HW_SERVERS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Cleaning $(server); $(GREY) \
		$(MAKE) -s -C $(server) clean;\
	)

# -----------------------------------------------------------------------------
