/*
  Author(s):  Sadovnikov Vladimir

  (C) SadKo (Sadovnikov And Company)
  (C) XSystem Kernel Team
*/

#ifndef _TMUTEX_H
#define _TMUTEX_H

#include <types.h>

class TMutex
{
 private:
  u8_t FLockItem;

 public:
  TMutex(void);
  
  bool lock(void);
  bool unlock(void);
  bool try_lock(void);
  bool is_locked(void);
};

#endif
