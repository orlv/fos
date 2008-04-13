/*
  kernel/main/system.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos/fos.h>
#include <stdio.h>
#include <fos/printk.h>
#include <fos/procman.h>

volatile size_t preempt_count;

SYSTEM::SYSTEM(register multiboot_info_t * mbi)
{
  this->mbi = mbi;
  user_int_handler = new Thread* [256];
  preempt.disable();
}

void SYSTEM::panic(register const char *fmt, ...)
{
  char panic_buf[512];
  system->cli();

  va_list args;
  va_start(args, fmt);
  vsprintf(panic_buf, fmt, args);
  va_end(args);

  printk("\n-------------------------------------------------------------------------------\n");
  printk("Kernel panic: %s \n", panic_buf);
  u16_t id = curPID();
  printk("ID: %d \n", id);
  if (procman && procman->curr)
    printk("PID: %d \n", procman->curr->item->process);
  printk("System Halted!\n");
  printk("--------------------------------------------------------------------------------\n");
  halt();
}

void SYSTEM::halt()
{
  while(1){
    system->cli();
    system->hlt();
  }
}

res_t SYSTEM::interrupt_attach(Thread *thread, u8_t n)
{
  if(!user_int_handler[n] && !ic->getHandler(n)){
    user_int_handler[n] = thread;
    return RES_SUCCESS;
  } else {
    return RES_FAULT;
  }
}

res_t SYSTEM::interrupt_detach(Thread *thread, u8_t n)
{
  if(user_int_handler[n] == thread){
    ic->mask(n);
    user_int_handler[n] = 0;
    return RES_SUCCESS;
  } else
    return RES_FAULT;
}
