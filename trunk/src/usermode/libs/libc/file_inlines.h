/*
 * Copyright (c) 2008 Sergey Gridassocv
 */

#ifndef FILE_INLINES_H
#define FILE_INLINES_H
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BUF_SIZE	512

static inline int format_mode(__fopen_fd *fd, const char *mode) {
	switch(mode[0]) {
	case 'r':
		fd->allow_read = 1;
		if(mode[1] == '+')
			fd->allow_write = 1;
		fd->open_mode = 0;
		break;
	case 'w':
		fd->allow_write = 1;
		if(mode[1] == '+')
			fd->allow_read = 1;
		fd->open_mode = O_CREAT | O_TRUNC;
		break;
	case 'a':
		fd->open_mode = O_CREAT | O_APPEND;
		fd->allow_write = 1;
		if(mode[1] == '+')
			fd->allow_read = 1;
		break;
	default:
		return -1;
	}
	return 0;

}

static inline FILE *funiopen(__fopen_fd *fd) {
	if(fd->handle < 0) {
		free(fd);
		return NULL;
	}
	fd->using_mutex = 0;

	fd->buf_size = BUF_SIZE;
	fd->buf_ptr = 0;
	fd->buf = malloc(fd->buf_size);
	if(!fd->buf) {
		free(fd);
		return NULL;
	}
	return fd;
}

static inline void internal_flush(__fopen_fd *fd) {
	if(fd->buf_ptr) {
		write(fd->handle, fd->buf, fd->buf_ptr);
		fd->buf_ptr = 0;
	}
}

static inline int buffering_write(__fopen_fd *fd, const char *ptr, size_t size) {
	if(size > fd->buf_size) {
		internal_flush(fd);
		return write(fd->handle, ptr, size);
	}
	if(size > fd->buf_size - fd->buf_ptr)
		internal_flush(fd);

	memcpy(fd->buf + fd->buf_ptr, ptr, size);
	fd->buf_ptr += size;
	char *cutptr;
	if((cutptr = memchr(ptr, '\n', size))) {
		internal_flush(fd);
	}
	return size;
}

#endif

