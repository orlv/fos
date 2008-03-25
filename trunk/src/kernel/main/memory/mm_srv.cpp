/*
  kernel/main/memory/mm_srv.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/printk.h>
#include <fos/process.h>
#include <fos/system.h>
#include <fos/pager.h>

void mm_srv()
{
  Thread *thread;
  struct message *msg = new message;

  while (1) {
    msg->recv_size = 0;
    msg->tid = 0;
    msg->flags = 0;

    receive(msg);

    switch(msg->arg[0]){
    case MM_CMD_MMAP:
      //printk("mm: mapping 0x%X bytes of memory to 0x%X, tid=%d\n", msg->arg[2], msg->arg[1], msg->tid);
      msg->arg[0] = (u32_t) THREAD(msg->tid)->process->memory->mmap(msg->arg[1] & ~0xfff, msg->arg[2], msg->arg[1] & 0xfff, msg->arg[3], 0);
      msg->send_size = 0;
      reply(msg);
      break;

    case MM_CMD_MUNMAP:
      //printk("mm: unmapping 0x%X bytes from 0x%X, tid=%d\n", msg->arg[2], msg->arg[1], msg->tid);
      thread = THREAD(msg->tid);
      if(msg->arg[1] > USER_MEM_BASE){
	msg->arg[0] = 1;
	thread->process->memory->munmap(msg->arg[1], msg->arg[2]);
      } else
	msg->arg[0] = -1;
      msg->send_size = 0;
      reply(msg);
      break;
      
    default:
      msg->arg[0] = RES_FAULT;
      msg->send_size = 0;
      reply(msg);
    }
  }
}
