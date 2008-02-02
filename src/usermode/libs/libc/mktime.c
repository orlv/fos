/*
 * from dietlibc
 */

#include <time.h>

extern long int timezone;
extern int daylight;

time_t mktime(struct tm *tm) {
  time_t x=timegm(tm);

  struct timezone tz;
  gettimeofday(0, &tz);
  timezone=tz.tz_minuteswest*60L;
  x+=timezone;

  return x;
}
