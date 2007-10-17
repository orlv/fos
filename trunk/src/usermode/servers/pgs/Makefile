include ../config.mk
include config.mk
include $(TARGET)al/config.mk
CFLAGS+=-Iinclude --std=c99
LDFLAGS+=$(LIBS_AL) $(addprefix $(TARGET)al/, $(OBJECTS_AL))
OBJECTS=main.o events.o cursor.o windowing.o list.o
MAKEFLAGS=-s
all: $(PGS_CUSTOM) al pgs
pgs: $(OBJECTS)
	echo Linking $@
	$(LD) $(LDFLAGS) $(LIB)/crt0.o $(OBJECTS) --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE) -o $@ -lc -lcpp -lfos

install:
	cp pgs $(INSTALLDIR)/$(OUTPUT)

clean:
	rm -f pgs $(OBJECTS)
	make -C $(TARGET)al clean

uninstall:
	rm -f $(INSTALLDIR)/pgs
al:
	make -C $(TARGET)al

