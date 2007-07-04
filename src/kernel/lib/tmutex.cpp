/*
  (C) SadKo (Sadovnikov And Company)
  (C) XSystem Kernel Team
*/

#include <tmutex.h>
#include <system.h>

TMutex::TMutex(void)
{
  FLockItem = 0;
}

bool TMutex::lock(void)
{
  int result;

  do {
    result = _xchg(&FLockItem, 1);
  } while (result!=0);

  return true;
}

bool TMutex::try_lock(void)
{
  return _xchg(&FLockItem, 1) == 0;
}

bool TMutex::is_locked(void)
{
  return FLockItem;
}

bool TMutex::unlock(void)
{
  return _xchg(&FLockItem, 0) == 0;
}
