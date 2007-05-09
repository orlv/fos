/*
  kernel/main/hal.h
  Copyright (C) 2007 Oleg Fedorov
*/

#include <hal.h>
#include <stdarg.h>
#include <vsprintf.h>
#include <stdio.h>
#include <system.h>
#include <tasks.h>

HAL::HAL(register multiboot_info_t * mbi)
{
  this->mbi = mbi;
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
  if (ProcMan->CurrentProcess)
    printk("PID: %d \n", ProcMan->CurrentProcess->pid);
  printk("System Halted!\n");
  printk("--------------------------------------------------------------------------------");
  halt();
}

void HAL::halt()
{
  hal->cli();
  while(1)
    hal->hlt();
}
