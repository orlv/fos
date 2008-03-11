/*
 * Copyright (c) 2008 Sergey Gridassocv
 */
#include <fos/fs.h>
#include <stdio.h>
#include <stdlib.h>

int fclose(FILE *fp) {
	__fopen_fd *fd = fp;
	if(!fd)
		return -1;
	fflush(fd);
	close(fd->handle);
	free(fd->buf);
	free(fd);
	return 0;
}
