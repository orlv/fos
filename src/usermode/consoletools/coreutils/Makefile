#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

TARGETS=ls uname
LDLIBS		=	-\( -lc -lfos -\)
all: $(TARGETS)

ls: ls.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
uname: uname.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)

install:
	cp -t $(INITRDDIR)/bin $(TARGETS)
uninstall:
	rm -f $(addprefix $(INITRDDIR)/bin/,$(TARGETS))
clean:
	rm -f *.o $(TARGETS)

.PHONY: $(TARGETS)