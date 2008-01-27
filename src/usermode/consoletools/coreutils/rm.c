/*
 * Copyright (c) 2007 - 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int force = 0;

void rm(char *file) {
	if(unlink(file) < 0 && !force)
		printf("rm: cannot remove `%s'\n", file);
}

void usage() {
	printf("Usage: rm [OPTION]... FILE]...\n"
#ifndef NOT_INCLUDE_HELP
	"Remove (unlink) the FILE(s)..\n\n"
	"  -f               ignore nonexistent files, never prompt\n"
        "  -h               display this help and exit\n"
	"  -V               output version information and exit\n\n"

	"Report bugs to <grindars@grindars.org.ru>.\n"
#endif
	);
}


void version() {
	printf("rm (FOS tiny coreutils) 0.1\n");
}

int main(int argc, char *argv[]) {
	int opt;
	while((opt = getopt(argc, argv, "hVf")) != -1) {
		switch(opt) {
		case 'h':
			usage();
			exit(0);
		case 'V':
			version();
			exit(0);
		case 'f':
			force = 1;
			break;
		default:
			usage();
			exit(1);
		}
	}
	if(optind >= argc) {
		usage();
		return 0;
	}
	for(int i = optind; i < argc; i++) {
		rm(argv[i]);
	}
	return 0;
}

