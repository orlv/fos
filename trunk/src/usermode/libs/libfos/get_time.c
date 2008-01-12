/*
 * Copyright (c) 2008 Sergey Gridassov
 */
#include <sys/rtc.h>
#include <fcntl.h>
#include <unistd.h>
int get_time(struct time *buf) {
	int handle = open("/dev/rtc", 0);
	if(!handle)
		return -1;

	if(read(handle, buf, sizeof(struct time)) < sizeof(struct time)) {
		close(handle);
		return -1;
	}
	close(handle);
	return 0;
}
