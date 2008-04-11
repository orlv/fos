/*
  drivers/i8253/i8253.h
  Copyright (C) 2006-2007 Oleg Fedorov
            (C)      2008 Sergey Gridassov
*/

#ifndef _i8253_H
#define _i8253_H

#include <types.h>
#include <time.h>
#include <fos/system.h>
#include <fos/drivers/interfaces/timer.h>

#define TIMER_IRQ_NUM 0

class i8253: public Timer {
 protected:
  u32_t _uptime;
 public:
  i8253();
  u32_t uptime();   /* получить показания таймера */

  void tick();
  void enable();
  void disable();
  void PeriodicalInt(int freq, void (*handler)());
};

#endif
