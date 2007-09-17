#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

OUTPUT		=	fjvm
OBJECTS		=	main.o jar.o alloc.o thread.o exec.o gc.o method.o native.o mytid.o hash.o synch.o heap.o 
OBJECTS        +=       metadata/class.o metadata/constpool/constpool.o metadata/constpool/constobject.o
OBJECTS        +=       zlib/match.o zlib/alloc.o zlib/zutil.o zlib/trees.o zlib/inftrees.o zlib/inflate.o zlib/inffast.o zlib/infback.o zlib/deflate.o zlib/crc32.o zlib/compress.o zlib/adler32.o
OBJECTS        +=       jit/objbuf.o jit/x86.o
LDLIBS		=	-\( -lc -lcpp -lfos -\)
JAR		=       jar

all:	$(OBJECTS)
	@echo \ Linking $(OUTPUT)
	@$(LD) $(LDFLAGS) -o $(OUTPUT) $(LIB)/crt0.o $(OBJECTS) $(LDLIBS) --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE)
	@$(JAR) cf classpath.jar classpath

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)
	cp classpath.jar $(INSTALLDIR)/classpath.jar

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)
	@rm -f classpath.jar

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)
	rm -f $(INSTALLDIR)/classpath.jar

#-----------------------------------------------------------------------------#
