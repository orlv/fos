/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "util.h"

void *load_file(const char *path) {
	int hndl = open(path, 0);
	if(hndl == -1)
		return NULL;
	int size = lseek(hndl, 0, SEEK_END);
	lseek(hndl, 0, SEEK_SET);

	void *buf = malloc(size);
	if(buf == NULL)
		return NULL;
	read(hndl, buf, size);
	close(hndl);
	return buf;
}
