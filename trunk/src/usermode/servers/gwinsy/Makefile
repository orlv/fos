#         FOS Graphics System
# Copyright (c) 2007 Sergey Gridassov

include $(ROOT)/config.mk
include $(SYSTEM_SRC)/mk/nicebuild.mk
CFLAGS+=-DVERSION=\"009\" -Iinc

OBJECTS= \
	main.o		events.o 	\
	cursor.o	windowing.o	\
	list.o		border.o	\
	context.o	font.o		\
	line.o		vbe.o		\
	loadfile.o	picture.o	\
	pixel.o		rect.o		\
	ipc.o
OBJECTS:=$(addprefix src/,$(OBJECTS))
MAKEFLAGS=-s
all: $(OBJECTS)
	echo \ Linking gwinsy
	$(LD) $(LDFLAGS) $(LIB)/crt0.o $^ --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE) -o gwinsy -lc -lcpp -lfos
	nm gwinsy | sort > gwinsy.sym
install:
	cp gwinsy $(INITRDDIR)/usr/bin

clean:
	rm -f gwinsy $(OBJECTS) gwinsy.sym

uninstall:
	rm -f $(INITRDDIR)/usr/bin/gwinsy

