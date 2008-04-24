/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <errno.h>
#include <stdio.h>

extern const char *sys_errlist[];
extern int sys_nerr;

void perror(const char *s) {
	if(s) 
		fprintf(stderr, "%s: ", s);

	if(errno < sys_nerr) 
		fprintf(stderr, "%s\n", sys_errlist[errno]);
	else
		fprintf(stderr, "error %d\n", errno);
}
