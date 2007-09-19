# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

include config.mk

ITEMS = crt0 libc libcpp libfos init shell test int16b servers

# -----------------------------------------------------------------------------

all: 
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Building $(item) ; $(GREY)\
		$(MAKE) -s -C $(item);\
	)

install:
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Install $(item) ; $(GREY)\
		$(MAKE) -s -C $(item) install;\
	)

uninstall:
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Uninstall $(item) ; $(GREY)\
		$(MAKE) -s -C $(item) uninstall;\
	)

clean:
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Cleaning $(item) ; $(GREY)\
		$(MAKE) -s -C $(item) clean;\
	)

# -----------------------------------------------------------------------------
