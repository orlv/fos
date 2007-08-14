/*
  fos/printk.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _FOS_PRINTK_H
#define _FOS_PRINTK_H

#include <types.h>

asmlinkage int printk(register const char *fmt, ...);

#endif
