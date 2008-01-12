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
	pixel.o		rect.o		
OBJECTS:=$(addprefix src/,$(OBJECTS))
MAKEFLAGS=-s
all: $(OBJECTS)
	echo \ Linking fgs
	$(LD) $(LDFLAGS) $(LIB)/crt0.o $^ --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE) -o fgs -lc -lcpp -lfos
	nm fgs | sort > fgs.sym
	strip fgs -R .comment
install:
	cp fgs $(INITRDDIR)/usr/bin

clean:
	rm -f fgs $(OBJECTS) fgs.sym

uninstall:
	rm -f $(INITRDDIR)/usr/bin/fgs

