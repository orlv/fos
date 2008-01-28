/*
 * Copyright (c) 2008 Sergey Gridassocv
 */

#include <stdio.h>
#include <fos/fs.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>

#include "file_inlines.h"

static inline int buffering_write(__fopen_fd *fd, const char *ptr, size_t size) {
	if(size > fd->buf_size) {
		internal_flush(fd);
		return write(fd->handle, ptr, size);
	}
	if(size > fd->buf_size - fd->buf_ptr)
		internal_flush(fd);

	memcpy(fd->buf + fd->buf_ptr, ptr, size);
	fd->buf_ptr += size;
	return size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	if(!stream)
		return 0;

	__fopen_fd *fd = stream;
	while(!mutex_try_lock(fd->using_mutex))
		sched_yield();

	int readed = buffering_write(fd, ptr, size * nmemb);

	mutex_unlock(fd->using_mutex);
	if(readed < 1)
		return 0;
	return readed / nmemb;

}
