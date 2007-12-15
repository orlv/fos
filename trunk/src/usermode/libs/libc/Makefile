#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libc.a
OBJECTS		=	close.o		\
			malloc.o	\
			exit.o		\
			lseek.o		\
			open.o		\
			read.o		\
			write.o		\
			sprintf.o	\
			vsprintf.o	\
			printf.o	\
			time.o		\
			stat.o		\
			fstat.o		\
			strspn.o \
			strcspn.o \
			strtok_r.o \
			strsep.o \
			random.o \
			opendir.o \
			getopt.o \
			readdir.o \
			closedir.o \
			rewinddir.o \
			seekdir.o \
			telldir.o

all:	$(OBJECTS)
	$(AR) rcs $(OUTPUT) $(OBJECTS)
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
