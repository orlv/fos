/*
  drivers/interfaces/timer.h
  Copyright (C) 2008 Sergey Gridassov
*/

#ifndef __TIMER_H
#define __TIMER_H
#include <types.h>

class Timer {
public:
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual u32_t uptime() = 0;
	virtual void PeriodicalInt(int freq, void (*handler)()) = 0;
	virtual void tick() = 0;
};
u32_t kuptime();    /* получить показания таймера */
void TimerCallSheduler();
#endif
