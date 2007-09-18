# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

include config.mk

ITEMS = crt0 libc libcpp libfos init shell test int16b servers

# -----------------------------------------------------------------------------

all: 
	@$(foreach item,$(ITEMS),\
		$(ECHO) ---------------------------------\> Building $(item);\
		$(MAKE) -s -C $(item);\
	)

install:
	@$(foreach item,$(ITEMS),\
		$(ECHO) ---------------------------------\> Install $(item);\
		$(MAKE) -s -C $(item) install;\
	)

uninstall:
	@$(foreach item,$(ITEMS),\
		$(ECHO) ---------------------------------\> Uninstall $(item);\
		$(MAKE) -s -C $(item) uninstall;\
	)

clean:
	@$(foreach item,$(ITEMS),\
		$(ECHO) ---------------------------------\> Cleaning $(item);\
		$(MAKE) -s -C $(item) clean;\
	)

# -----------------------------------------------------------------------------
