/*
  kernel/drivers/apic/timer.cpp
  Copyright (C) 2008 Sergey Gridassov
*/

#include <fos/system.h>
#include "timer.h"

APICTimer::APICTimer(volatile u32_t *regs) {
	apic_regs = regs;
	_uptime = 0;
}

void APICTimer::enable() {
  system->panic("APICTimer: enable() not implemented\n");
}

void APICTimer::disable() {
  system->panic("APICTimer: enable() not implemented\n");
}

void APICTimer::PeriodicalInt(int freq, void (*handler)()) {
  system->panic("APICTImer: PeriodicalInt(%d, 0x%X) not implemented\n", freq, handler);
}
