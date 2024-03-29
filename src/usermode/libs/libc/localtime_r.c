/*
 * from dietlibc
 */


#include <time.h>
#include <sys/time.h>

extern long int timezone;
extern int daylight;

struct tm* localtime_r(const time_t* t, struct tm* r) {
  time_t tmp;
  struct timezone tz;
  gettimeofday(0, &tz);
  timezone=tz.tz_minuteswest*60L;
  tmp=*t+timezone;
  return gmtime_r(&tmp,r);
}
