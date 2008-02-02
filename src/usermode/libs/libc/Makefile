#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libc.a
OBJECTS		=	close.o		malloc.o	exit.o		lseek.o		\
			open.o		read.o		write.o		sprintf.o	\
			vsprintf.o	printf.o	time.o		stat.o		\
			fstat.o		strspn.o	strcspn.o	strtok_r.o 	\
			strsep.o	random.o 	opendir.o	getopt.o 	\
			readdir.o	closedir.o	rewinddir.o	seekdir.o 	\
			telldir.o	strtoul.o	__add_to_environ.o		\
			setenv.o	unsetenv.o	getenv.o	getcwd.o	\
			fgets.o		chdir.o		putenv.o	realpath.o	\
			unlink.o	fopen.o		freopen.o	fclose.o	\
			fflush.o	fread.o		fdopen.o	fwrite.o	\
			fseek.o		rewind.o	ftell.o		setjmp.o	\
			sigjmp.o	__longjmp.o	longjmp.o	fputc.o		\
			vfprintf.o	fgetc.o		unlocked_fputc.o		\
			unlocked_fwrite.o		fprintf.o	snprintf.o	\
			vsnprintf.o

include $(SYSTEM_SRC)/mk/trg_lib.mk


fopen.o fflush.o fdopen.o fwrite.o unlocked_fwrite.o: file_inlines.h