/*
 * kernel/main/main.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <multiboot.h>
#include <fos/drivers/char/tty/tty.h>
#include <fos/mm.h>
#include <string.h>
#include <fos/fos.h>
#include <fos/printk.h>
#include <fos/drivers/char/timer/timer.h>
#include <fos/hal.h>
#include <fos/traps.h>
#include <vsprintf.h>
#include <stdarg.h>
#include <fos/drivers/fs/modulefs/modulefs.h>
#include <fos/fs.h>

TTY *stdout;
Timer *SysTimer;
HAL *hal;
ModuleFS *initrb; /* Модули, загруженные GRUB */

void out_banner()
{
  extern const string version;
  extern u32_t build;

  printk(" FOS %s (%s) \n"						\
	 "-------------------------------------------------------------------------------\n" \
	 " Copyright (C) 2004-2007 Oleg Fedorov                      http://fos.osdev.ru/\n" \
	 "-------------------------------------------------------------------------------\n",
	 version, build);
}

asmlinkage void init()
{
  init_memory();

  hal->cli();
  hal->pic = new PIC;

  hal->gdt = new GDT;
  hal->idt = new IDT;

  setup_idt();
  hal->sti();

  /*  TTY *tty1 = new TTY(80, 25);
  tty1->set_text_color(WHITE);
  stdout = tty1;*/

  out_banner();

  printk("Memory size: %d Kb, free %dK (%dK high/%dK low)\n", hal->pages_cnt*4, hal->free_pages.read()*4 + hal->free_pages_DMA16.read()*4, hal->free_pages.read()*4, hal->free_pages_DMA16.read()*4);

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
