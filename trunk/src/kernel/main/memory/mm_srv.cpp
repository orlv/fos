/*
  kernel/main/memory/mm_srv.cpp
  Copyright (C) 2007 Oleg Fedorov

  *(Tue Mar 25 12:21:50 2008) переделан на использование NSI
*/

#include <fos/mm.h>
#include <fos/printk.h>
#include <fos/process.h>
#include <fos/system.h>
#include <fos/pager.h>
#include <fos/nsi.h>

int mm_mmap(struct message *msg)
{
  //printk("mm: mapping 0x%X bytes of memory to 0x%X, tid=%d\n", msg->arg[2], msg->arg[1], msg->tid);
  msg->arg[0] = (u32_t) THREAD(msg->tid)->process->memory->mmap(msg->arg[1] & ~0xfff,
								msg->arg[2],
								msg->arg[1] & 0xfff,
								msg->arg[3], 0);
  msg->send_size = 0;
  return 1;
}

int mm_munmap(struct message *msg)
{
  //printk("mm: unmapping 0x%X bytes from 0x%X, tid=%d\n", msg->arg[2], msg->arg[1], msg->tid);
  if (msg->arg[1] > USER_MEM_BASE) {
    msg->arg[0] = 1;
    THREAD(msg->tid)->process->memory->munmap(msg->arg[1],
					      msg->arg[2]);
  } else
    msg->arg[0] = -1;

  msg->send_size = 0;
  return 1;
}

void mm_srv()
{
  nsi_t *interface = new nsi_t();

  interface->add(MM_CMD_MMAP, &mm_mmap);
  interface->add(MM_CMD_MUNMAP, &mm_munmap);
  
  while (1) {
    interface->wait_message();
  };
}
