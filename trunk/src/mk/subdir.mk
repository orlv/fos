# -----------------------------------------------------------------------------
# Copyright (C) 2006-2008 Oleg Fedorov
# -----------------------------------------------------------------------------

all: 
	$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Building $(item) ; $(GREY)\
		$(MAKE) ROOT=$(ROOT)/.. -C $(item) all || exit 1;\
	)
 
 
install:
	$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Install $(item) ; $(GREY)\
		$(MAKE) ROOT=$(ROOT)/.. -C $(item) install || exit 1;\
	)
 
uninstall:
	$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Uninstall $(item) ; $(GREY)\
		$(MAKE) ROOT=$(ROOT)/.. -C  $(item) uninstall || exit 1;\
	)
 
clean:
	$(foreach item,$(ITEMS),\
		$(YELLOW) $(ECHO) ---------------------------------\> Cleaning $(item) ; $(GREY)\
		$(MAKE)  ROOT=$(ROOT)/.. -C $(item) clean || exit 1;\
	)
 
