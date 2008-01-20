/*
  include/fos/signal.h
  Copyright (C) 2008 Oleg Fedorov
*/

#ifndef _FOS_SIGNAL_H
#define _FOS_SIGNAL_H

#include <types.h>

struct signal {
  u32_t n;    /* номер сигнала  */
  u32_t data; /* данные сигнала */
};

#endif
