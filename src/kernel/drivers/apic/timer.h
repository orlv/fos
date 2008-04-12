/*
  kernel/drivers/apic/timer.h
  Copyright (C) 2008 Sergey Gridassov
*/

#ifndef __APIC_TIMER_H
#define __APIC_TIMER_H
#include <fos/drivers/interfaces/timer.h>


class APICTimer: public Timer {
private:
	volatile u32_t *apic_regs;
	u32_t _uptime;
public:
	APICTimer(volatile u32_t *regs);
	void tick();
	void enable();
	void disable();
	void PeriodicalInt(int freq, void (*handler)());
	u32_t uptime();
};

#endif


