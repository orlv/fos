/*
 * Copyright (c) 2008 Sergey Gridassocv
 */
#include <fos/fs.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "file_inlines.h"

FILE *fdopen(int fd, const char *mode) {
	__fopen_fd *new = malloc(sizeof(__fopen_fd));
	if(!new)
		return NULL;

	if(format_mode(new, mode) < 0) {
		free(new);
		return NULL;
	}
	new->handle = fd;

	return funiopen(new);
}

