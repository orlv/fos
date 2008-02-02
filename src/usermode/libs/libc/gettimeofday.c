/*
 * Copyright (c) 2008 Sergey Gridassov
 *
 */
#include <sys/time.h>
#include <sys/rtc.h>
#include <time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	struct time buf;
	if(get_time(&buf) < 0)
		return -1;
	if(!tv) 
		return 0;
	struct tm tm;
	tm.tm_sec = buf.sec;
	tm.tm_min = buf.min;
	tm.tm_hour = buf.hour;
	tm.tm_mday = buf.day;
	tm.tm_mon = buf.month - 1;
	tm.tm_year = buf.year - 1900;
	time_t time = mktime(&tm);
	if(time == (time_t) -1)
		return -1;
	tv->tv_sec = time; 
	tv->tv_usec = 0;
	return 0;

}
