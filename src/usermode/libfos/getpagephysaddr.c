/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/syscall.h>

off_t getpagephysaddr(off_t pageaddr)
{
  return sys_call(_FOS_GET_PAGE_PHYS_ADDR, pageaddr);
}
