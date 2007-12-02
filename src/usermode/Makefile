# -----------------------------------------------------------------------------
# Copyright (C) 2006-2007 Oleg Fedorov
# -----------------------------------------------------------------------------

include config.mk

ITEMS = crt0 libc libcpp libfos init shell test int16b ipctest # test2
#ITEMS += ext2
ITEMS += servers

# -----------------------------------------------------------------------------

all: 
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Building $(item) ; $(GREY)\
		$(MAKE) -s -C $(item) || exit 1;\
	)


install:
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Install $(item) ; $(GREY)\
		$(MAKE) -s -C $(item) install || exit 1;\
	)

uninstall:
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Uninstall $(item) ; $(GREY)\
		$(MAKE) -s -C $(item) uninstall || exit 1;\
	)

clean:
	@$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Cleaning $(item) ; $(GREY)\
		$(MAKE) -s -C $(item) clean || exit 1;\
	)

# -----------------------------------------------------------------------------
