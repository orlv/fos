/*
 * kernel/main/main.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#define ENABLE_APIC	0

#include <multiboot.h>
#include <fos/drivers/tty/tty.h>
#include <fos/mm.h>
#include <string.h>
#include <fos/fos.h>
#include <fos/printk.h>
#include <fos/drivers/i8253/i8253.h>
#include <fos/drivers/pic/pic.h>
#include <fos/traps.h>
#include <stdarg.h>
#include <fos/drivers/modulefs/modulefs.h>
#include <fos/drivers/apic/apic.h>
#include <fos/fs.h>

TTY *stdout;
Timer *SysTimer;
SYSTEM *system;
ModuleFS *initrb; /* Модули, загруженные GRUB */

void init_memory();

void out_banner()
{
  extern const char *version;
  extern u32_t build;

  printk(" FOS %s (%s) \n"						\
	 "-------------------------------------------------------------------------------\n" \
	 " Copyright (C) 2004-2008 Oleg Fedorov                      http://fos.osdev.ru/\n" \
	 " Copyright (C) 2007-2008 Serge Gridassov \n" \
	 "-------------------------------------------------------------------------------\n",
	 version, build);
}

asmlinkage void init()
{
  init_memory();


  system->cli();

  out_banner();

  system->cpuid = new CPUID();

  system->gdt = new GDT;
  system->idt = new IDT;

  setup_idt();

  if(system->cpuid->features_edx & FEATURE_APIC && ENABLE_APIC) {
    printk("Using APIC\n");
    system->smp = new SMP;
    system->ic = new APIC(0);
    SysTimer = system->ic->getTimer();
//    system->sti();
  } else {
    printk("Using legacy ISA PIC and timer\n");
    system->ic = new PIC;
    SysTimer = new i8253;
    system->sti();
  }



  /*  TTY *tty1 = new TTY(80, 25);
  tty1->set_text_color(WHITE);
  stdout = tty1;*/


  printk("Memory size: %d Kb, free %dK (%dK high/%dK low)\n", system->pages_cnt*4, system->free_pages*4 + system->free_pages_DMA16*4, system->free_pages*4, system->free_pages_DMA16*4);
  extern multiboot_info_t *__mbi;
  initrb = new ModuleFS(__mbi);
  system->procman = new TProcMan;


  SysTimer->PeriodicalInt(1000, TimerCallSheduler);
  SysTimer->enable();

  printk("Kernel: start task switching\n");
 
  system->preempt.reset();  /* сбросим счетчик на 1 */
  system->preempt.enable();

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
      printk("Memory size: %d Kb, free %dK (%dK high/%dK low), heap_free=%d \n", system->pages_cnt*4, system->free_pages.read()*4 + system->free_lowpages.read()*4, system->free_pages.read()*4, system->free_lowpages.read()*4, heap_free);
      i=0;
    }
    sched_yield();
  }
#else
  while(1) { printk(".");
    system->hlt(); }
#endif
}
