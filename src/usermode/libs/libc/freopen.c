/*
 * Copyright (c) 2008 Sergey Gridassocv
 */

#include <stdio.h>

FILE *freopen(const char *path, const char *mode, FILE *stream) {
	fclose(stream);
	return fopen(path, mode);
}

