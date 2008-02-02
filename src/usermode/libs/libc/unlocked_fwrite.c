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

size_t unlocked_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	if(!stream)
		return 0;

	__fopen_fd *fd = stream;

	int readed = buffering_write(fd, ptr, size * nmemb);

	if(readed < 1)
		return 0;
	return readed / nmemb;

}
