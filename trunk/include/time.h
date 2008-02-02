/*
  include/time.h
  Copyright (C) 2006 Oleg Fedorov
                2008 Sergey Gridassov
*/

#ifndef _TIME_H
#define _TIME_H

#include <types.h>
#include <sys/time.h>

struct tm {
  int tm_sec;   /* seconds after the minute [0, 59] */
  int tm_min;   /* minutes after the hour [0, 59] */
  int tm_hour;  /* hours since midnight [0, 23] */
  int tm_mday;	/* day of the month [1, 31] */
  int tm_mon;   /* months since January [0, 11] */
  int tm_year;	/* years since 1900 */
  int tm_wday;	/* days since Sunday [0, 6] */
  int tm_yday;	/* days since January 1 [0, 365] */
  int tm_isdst;	/* Daylight Saving Time flag */
};

userlinkage struct tm *gmtime(const time_t * timep);
userlinkage struct tm *gmtime_r(const time_t *timep, struct tm *r);
userlinkage struct tm* localtime(const time_t* t);
userlinkage struct tm* localtime_r(const time_t* t, struct tm* r);
userlinkage time_t mktime(struct tm *tm);
#define timelocal(a) mktime(a)
userlinkage time_t timegm(struct tm *tm);
userlinkage time_t time(time_t *t);

extern int daylight;
extern long timezone;
#endif
