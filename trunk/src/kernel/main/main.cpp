/*
 * kernel/main/main.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <multiboot.h>
#include <drivers/char/tty/tty.h>
#include <mm.h>
#include <drivers/block/vga/vga.h>
#include <string.h>
#include <system.h>
#include <stdio.h>
#include <drivers/char/timer/timer.h>
#include <hal.h>
#include <traps.h>
#include <vsprintf.h>
#include <stdarg.h>
#include <drivers/fs/modulefs/modulefs.h>
#include <fs.h>
#include <drivers/char/usertty/usertty.h>

TTY *stdout;
Timer *SysTimer;
HAL *hal;
ModuleFS *initrb; /* Модули, загруженные GRUB */

void out_banner()
{
  extern const string version;
  extern u32_t build;
  extern const string compile_date, compile_time;

  printk(" FOS OS. Revision %s. Build #%d %s %s \n"			\
	 "-------------------------------------------------------------------------------\n" \
	 " Copyright (C) 2004-2007 Oleg Fedorov                      http://fos.osdev.ru/\n" \
	 "-------------------------------------------------------------------------------\n",
	 version, build, compile_date, compile_time);
}

asmlinkage tid_t exec(const char * filename)
{
  size_t len = strlen(filename);
  if(len+1 > MAX_PATH_LEN)
    return 0;
  message msg;
  msg.a0 = PROCMAN_CMD_EXEC;
  msg.send_buf = filename;
  msg.send_size = len + 1;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  return send(&msg);
}

asmlinkage void init()
{
  init_memory();

  hal->cli();
  hal->pic = new PIC;
  hal->pic->remap(0x20, 0x28);

  int i;
  for(i = 0; i < 16; i++)
    hal->pic->mask(i);

  hal->gdt = new GDT;
  hal->idt = new IDT;

  setup_idt();
  hal->sti();

  VGA *con = new VGA;
  TTY *tty1 = new TTY(80, 25);

  tty1->stdout = con;
  tty1->set_text_color(WHITE);

  stdout = tty1;

  out_banner();
  printk("Memory size: %d Kb, free %dK (%dK high/%dK low)\n", hal->pages_cnt*4, hal->free_pages.read()*4 + hal->free_lowpages.read()*4, hal->free_pages.read()*4, hal->free_lowpages.read()*4);

  hal->procman = new TProcMan;

  SysTimer = new Timer;
  extern multiboot_info_t *__mbi;
  initrb = new ModuleFS(__mbi);
  
  printk("Kernel: start task switching\n");
 
  hal->mt_reset();  /* сбросим счетчик на 1 */
  hal->mt_enable();

  sched_yield();

  printk("-------------------------------------------------------------------------------\n" \
	 "All OK. Main kernel procedure done.\n"			\
	 "-------------------------------------------------------------------------------\n");

#if 0
  extern size_t volatile heap_free;
  i=0;
  while(1){
    i++;
    if(i == 0x60){
      printk("Memory size: %d Kb, free %dK (%dK high/%dK low), heap_free=%d \n", hal->pages_cnt*4, hal->free_pages.read()*4 + hal->free_lowpages.read()*4, hal->free_pages.read()*4, hal->free_lowpages.read()*4, heap_free);
      i=0;
    }
    sched_yield();
  }
#else
  while(1)
    sched_yield();
#endif
}