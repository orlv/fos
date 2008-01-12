#ifndef RTC_H
#define RTC_H
#include <types.h>
struct time {
  u16_t year;
  u8_t month;
  u8_t day;
  u16_t hour;
  u8_t min;
  u8_t sec;
} __attribute__ ((packed));
int get_time(struct time *buf);
#endif
