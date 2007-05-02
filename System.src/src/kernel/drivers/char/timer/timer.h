/*
  drivers/char/timer.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _TIMER_H
#define _TIMER_H

#include <types.h>
#include <time.h>

class TTime {
protected:
  u32_t _uptime;

public:
  TTime();
  void tick();
  u32_t uptime();		/* получить показания таймера */
};

//extern "C" void timer_handler(u16_t cs);

u32_t uptime();			/* получить показания таймера */

#endif
