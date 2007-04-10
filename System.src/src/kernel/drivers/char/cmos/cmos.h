/*
  drivers/char/cmos/cmos.h
  Coptright (C) 2006 Oleg Fedorov
*/

#ifndef __CMOS_H
#define __CMOS_H

#include <tinterface.h>
#include <types.h>

struct time {
  u16_t year;
  u8_t month;
  u8_t day;
  u16_t hour;
  u8_t min;
  u8_t sec;
} __attribute__ ((packed));

class CMOS:public Tinterface {
protected:
  u8_t read(u8_t data);
  void write(u8_t data, u8_t reg);

public:
   CMOS();

  size_t read(off_t offset, void *buf, size_t count);

};

#endif
