/*
  drivers/char/timer.h
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#ifndef _TIMER_H
#define _TIMER_H

#include <types.h>
#include <time.h>
#include <fos/system.h>

#define TIMER_IRQ_NUM 0

class Timer {
protected:
  u32_t _uptime;

public:
  Timer();
  void tick();
  u32_t uptime();   /* получить показания таймера */

  inline void enable()
  {
    system->pic->unmask(TIMER_IRQ_NUM);
  }

  inline void disable()
  {
    system->pic->mask(TIMER_IRQ_NUM);
  }
};

u32_t kuptime();    /* получить показания таймера */

#endif
