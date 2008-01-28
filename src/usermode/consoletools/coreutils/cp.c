/*
 * Copyright (c) 2007 - 2008 Sergey Gridassov
 *
 * TODO:
 *  - Копирование такого типа: cp /somedir/somefile /anotherdir
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

 
void usage();
void version();
void cp(char *src, char *dest);

int main(int argc, char *argv[]) {
	int opt;
	while((opt = getopt(argc, argv, "hV")) != -1) {
		switch(opt) {
		case 'h':
			usage();
			exit(0);
		case 'V':
			version();
			exit(0);
		default:
			usage();
			exit(1);
		}
	}
	if(optind >= argc - 1) {
		usage();
		return 0;
	}
	cp(argv[optind], argv[optind + 1]);

	return 0;
}

void usage() {
	printf("Usage: cp [OPTION]... SOURCE DEST\n"
#ifndef NOT_INCLUDE_HELP
	"Copy SOURCE to DEST\n\n"
        "  -h               display this help and exit\n"
	"  -V               output version information and exit\n\n"

	"Report bugs to <grindars@grindars.org.ru>.\n"
#endif
	);
}

void cp(char *src, char *dest) {
	int hndl2 = open(dest, O_CREAT);
	int hndl = open(src, 0);
	if(hndl < 0) {
		printf("cp: can't open `%s'\n", src);
		return;
	}
	if(hndl2 < 0) {
		printf("cp: can't open `%s'\n", dest);
		close(hndl);
		return;
	}
  	int size = lseek(hndl, 0, SEEK_END);
  	lseek(hndl, 0, SEEK_END);
	if(size) {
		char *buf = malloc(size);
		read(hndl, buf, size);
		write(hndl2, buf, size);
		free(buf);
	}
	close(hndl);
	close(hndl2);
}
void version() {
	printf("cp (FOS tiny coreutils) 0.1\n");
}
