/*
  kernel/main/hal.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <hal.h>
#include <stdarg.h>
#include <vsprintf.h>
#include <stdio.h>
#include <system.h>
#include <procman.h>

HAL::HAL(register multiboot_info_t * mbi)
{
  this->mbi = mbi;
  user_int_handler = new Thread* [256];
  mt_disable();
}

void HAL::panic(register const char *fmt, ...)
{
  char panic_buf[512];
  hal->cli();

  va_list args;
  va_start(args, fmt);
  vsprintf(panic_buf, fmt, args);
  va_end(args);

  printk("\n--------------------------------------------------------------------------------");
  printk("Kernel panic: %s \n", panic_buf);
  u16_t id = curPID();
  printk("ID: %d \n", id);
  if (procman->CurrentThread)
    printk("PID: %d \n", procman->CurrentThread->process);
  printk("System Halted!\n");
  printk("--------------------------------------------------------------------------------");
  halt();
}

void HAL::halt()
{
  while(1){
    hal->cli();
    hal->hlt();
  }
}

res_t HAL::interrupt_attach(Thread *thread, u8_t n)
{
  if(!user_int_handler[n]){
    user_int_handler[n] = thread;
    return RES_SUCCESS;
  } else {
    return RES_FAULT;
  }
}

res_t HAL::interrupt_detach(Thread *thread, u8_t n)
{
  if(user_int_handler[n] == thread){
    user_int_handler[n] = 0;
    return RES_SUCCESS;
  } else
    return RES_FAULT;
}
