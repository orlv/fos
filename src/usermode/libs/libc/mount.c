/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <fos/fos.h>
#include <sys/mount.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data) {

	if(source == NULL || source[0] == 0 || target == NULL || target[0] == 0) {
		errno = EFAULT;
		return -1;
	}

	// [source] [target] <data...>\0
	size_t len = strlen(source) + 1 + strlen(target) + 1;
	if(data)
		len += strlen(data);

	char *args = malloc(len);
	if(data)
		snprintf(args, len, "%s %s %s", source, target, data);
	else
		snprintf(args, len, "%s %s", source, target);

	len = sizeof("/sbin/fsd_") + strlen(filesystemtype);
	
	char *server = malloc(len);
	snprintf(server, len, "/sbin/fsd_%s", filesystemtype);
	
	tid_t tid = exec(server, args);
	free(server);
	free(args);

	if(tid == 0) {
		errno = ENODEV;
		return -1;
	}

	int timeout = 5000; // 5 сек

	do {
		if(ping(target) == 0)
			break;
		u32_t start_up = uptime();
		while(uptime() == start_up) sched_yield();

	} while(timeout--);

	if(timeout == 0) {
		errno = ENXIO;
		return -1;
	}

	return 0;
}
