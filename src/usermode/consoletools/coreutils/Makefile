#----------------------------------------------------------------------------#
#	Copyright (C) 2007-2008 Sergey Gridassov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk
include $(SYSTEM_SRC)/mk/nicebuild.mk

TARGETS=ls uname cat echo rm cp mv
LDLIBS		=	-\( -lc -lfos -\)

# если надо место сэкономить
# CFLAGS += -DNOT_INCLUDE_HELP
all: $(TARGETS)

ls: ls.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
uname: uname.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
cat: cat.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
echo: echo.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
rm: rm.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
cp: cp.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
mv: mv.o
	$(LD) $(LDFLAGS) -o $@ $(LIB)/crt0.o $^ $(LDLIBS)
install:
	cp -t $(INITRDDIR)/bin $(TARGETS)
uninstall:
	rm -f $(addprefix $(INITRDDIR)/bin/,$(TARGETS))
clean:
	rm -f *.o $(TARGETS)

.PHONY: $(TARGETS)