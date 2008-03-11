/*
 * Copyright (c) 2008 Sergey Gridassocv
 */

#include <fos/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include "file_inlines.h"

int fseek(FILE *stream, long offset, int whence) {
	if(!stream)
		return 0;

	__fopen_fd *fd = stream;

	while(!mutex_try_lock(&fd->using_mutex))
		sched_yield();

	internal_flush(fd);
	int ret = lseek(fd->handle, offset, whence);
	mutex_unlock(&fd->using_mutex);
	return ret == -1 ? -1 : 0;

} 
