/*
 * Copyright (c) 2008 Sergey Gridassocv
 */

#include <fos/fs.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "file_inlines.h"

FILE *fopen(const char *path, const char *mode) {

	__fopen_fd *new = malloc(sizeof(__fopen_fd));
	if(!new)
		return NULL;

	if(format_mode(new, mode) < 0) {
		free(new);
		return NULL;
	}
	new->handle = open(path, new->open_mode);

	return funiopen(new);
	
} 


