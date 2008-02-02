/*
 * Copyright (C) 2008 Sergey Gridassov
 */ 

#ifndef TIME_H
#define TIME_H
#include <types.h>
typedef u32_t time_t;
typedef u32_t suseconds_t;
struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
