/*
 * Copyright (c) 2008 Sergey Gridassocv
 */
#include <fos/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include "file_inlines.h"
int fflush(FILE *fp) {
	if(!fp)
		return 0;

	__fopen_fd *fd = fp;

	while(!mutex_try_lock(fd->using_mutex))
		sched_yield();

	internal_flush(fd);

	mutex_unlock(fd->using_mutex);
	return 0;
}
