/*
	drivers/char/cmos/cmos.cpp
	Copyright (C) 2006 Oleg Fedorov
*/

#include <types.h>
#include "cmos.h"
#include <tinterface.h>
#include <hal.h>
#include <stdio.h>

static u32_t DecodeBCD(u32_t bcd)
{
  u32_t dec = 0;
  u32_t mask = 0xf0000000;

  for (u32_t i = 0; i < 32; i += 4) {
    dec = 10 * dec + ((bcd & mask) >> (28 - i));
    mask >>= 4;
  }
  return dec;
}

CMOS::CMOS()
{
  struct time *time = new struct time;
  read(0, time, sizeof(struct time));
  printk("System Time: %d-%02d-%02d %02d:%02d:%02d\n", time->year, time->month,
	 time->day, time->hour, time->min, time->sec);
  delete time;
}

size_t CMOS::read(off_t offset, void *buf, size_t count)
{
  if (count < sizeof(struct time))
    return 0;

  hal->outportb(0x70, 0x0b);
  u8_t i = hal->inportb(0x71);
  i &= 0xffff - 4;
  hal->outportb(0x71, i);

  ((struct time *)buf)->year = DecodeBCD(read(0x32)) * 100;	/* Старшие цифры года */
  ((struct time *)buf)->year += DecodeBCD(read(9));	/* Младшие цифры года */

  ((struct time *)buf)->month = DecodeBCD(read(8));	/* Месяц */
  ((struct time *)buf)->day = DecodeBCD(read(7));	/* День */

  ((struct time *)buf)->hour = DecodeBCD(read(4));	/* Час */
  ((struct time *)buf)->min = DecodeBCD(read(2));	/* Минута */
  ((struct time *)buf)->sec = DecodeBCD(read(0));	/* Секунда */

  return sizeof(struct time);
}

u8_t CMOS::read(u8_t data)
{
  hal->outportb(0x70, data);
  return hal->inportb(0x71);
}

void CMOS::write(u8_t data, u8_t reg)
{
  hal->outportb(0x70, data);
  hal->outportb(0x71, reg);
}
