/*
  drivers/interfaces/timer.cpp
  Copyright (C) 2008 Sergey Gridassov
*/

#include <fos/printk.h>
#include <fos/system.h>
#include <types.h>
#include "timer.h"

u32_t kuptime()
{
  extern Timer *SysTimer;
  return SysTimer->uptime();
}

void TimerCallSheduler()
{
  if (system->preempt.status() && sched_ready())
    sched_yield();
}

Timer::~Timer() {

}