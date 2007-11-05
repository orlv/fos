/*
  Author(s):  Sadovnikov Vladimir

  (C) SadKo (Sadovnikov And Company)
  (C) XSystem Kernel Team
*/

#ifndef _CPP_TMUTEX_H
#define _CPP_TMUTEX_H

#include <types.h>

class TMutex
{
 private:
  volatile u8_t FLockItem;

 public:
  TMutex(void);
  
  void lock(void);
  bool unlock(void);
  bool try_lock(void);
  bool is_locked(void);
};

#endif
