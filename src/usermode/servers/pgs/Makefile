include ../config.mk
include config.mk
include $(TARGET)al/config.mk
CFLAGS+=-Iinclude --std=c99 -DVERSION=$(VERSION)
LDFLAGS+=$(LIBS_AL) $(addprefix $(TARGET)al/, $(OBJECTS_AL))

OBJECTS=main.o events.o cursor.o windowing.o list.o
MAKEFLAGS=-s
all: $(PGS_CUSTOM) al $(OBJECTS)
	echo \ Linking pgs
	$(LD) $(LDFLAGS) $(LIB)/crt0.o $(OBJECTS) --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE) -o pgs -lc -lcpp -lfos
	nm pgs | sort > pgs.sym
	strip -R .comment pgs

install:
	cp pgs $(INITRDDIR)/usr/bin/$(OUTPUT)

clean:
	rm -f pgs $(OBJECTS)
	make -C $(TARGET)al clean

uninstall:
	rm -f $(INITRDDIR)/usr/bin/pgs
al:
	make -C $(TARGET)al

