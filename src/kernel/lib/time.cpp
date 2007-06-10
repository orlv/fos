/*
  kernel/lib/time.cpp
  Временно позаимствовано из Minix.
*/

#include <time.h>
#include <mm.h>

#define YEAR0           1900	/* the first year */
#define EPOCH_YR        1970	/* EPOCH = Jan 1 1970 00:00:00 */
#define SECS_DAY        (24L * 60L * 60L)
#define LEAPYEAR(year)  (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)  (LEAPYEAR(year) ? 366 : 365)
//#define FIRSTSUNDAY(timp)       (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
//#define FIRSTDAYOF(timp)        (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)
#define TIME_MAX        ULONG_MAX
#define ABB_LEN         3

/*
static const string _days[] =
{
  "Sunday", "Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday"
};

static const string _months[] =
{
  "January", "February", "March",
  "April", "May", "June",
  "July", "August", "September",
  "October", "November", "December"
};
*/

//extern long _timezone;
//extern long _dst_off;
//extern int _daylight;
//extern char *_tzname[2];

static const u32_t _ytab[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

struct tm *gmtime(const time_t * timer)
{
  struct tm *timep = new(struct tm);

  time_t time = *timer;
  u32_t dayclock, dayno;
  u32_t year = EPOCH_YR;

  dayclock = (u32_t) time % SECS_DAY;
  dayno = (u32_t) time / SECS_DAY;

  timep->tm_sec = dayclock % 60;
  timep->tm_min = (dayclock % 3600) / 60;
  timep->tm_hour = dayclock / 3600;
  timep->tm_wday = (dayno + 4) % 7;	/* day 0 was a thursday */
  while (dayno >= YEARSIZE(year)) {
    dayno -= YEARSIZE(year);
    year++;
  }
  timep->tm_year = year - YEAR0;
  timep->tm_yday = dayno;
  timep->tm_mon = 0;
  while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
    dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
    timep->tm_mon++;
  }
  timep->tm_mday = dayno + 1;
  timep->tm_isdst = 0;

  return timep;
}
