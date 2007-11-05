/*
  (C) SadKo (Sadovnikov And Company)
  (C) XSystem Kernel Team
*/

#include <c++/tmutex.h>
#include <asm/xchg.h>

TMutex::TMutex(void)
{
  FLockItem = 0;
}

void TMutex::lock(void)
{
  while (xchg(&FLockItem, 1) != 0);
}

bool TMutex::try_lock(void)
{
  return xchg(&FLockItem, 1) == 0;
}

bool TMutex::is_locked(void)
{
  return FLockItem;
}

bool TMutex::unlock(void)
{
  return xchg(&FLockItem, 0) == 0;
}
