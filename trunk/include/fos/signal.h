/*
  include/fos/signal.h
  Extended signals for FOS
  Copyright (C) 2008 Oleg Fedorov
*/

#ifndef _FOS_SIGNAL_H
#define _FOS_SIGNAL_H

#include <types.h>

struct ext_signal {
  u32_t data; /* данные сигнала */
  u32_t n;    /* номер сигнала  */
};

#endif
