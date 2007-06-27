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

  printk(" FOS OS. Revision %s. Build #%s %s %s \n"			\
	 "--------------------------------------------------------------------------------" \
	 " Copyright (C) 2004-2007 Oleg Fedorov                      http://fos.osdev.ru/ " \
	 "--------------------------------------------------------------------------------",
	 version, build, compile_date, compile_time);
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
  printk("Memory size: %d Kb, free %dK (%dK high/%dK low)\n", hal->pages_cnt*4, atomic_read(&hal->free_pages)*4 + atomic_read(&hal->free_lowpages)*4, atomic_read(&hal->free_pages)*4, atomic_read(&hal->free_lowpages)*4);

  hal->ProcMan = new TProcMan;
  SysTimer = new Timer;

  printk("Kernel: start task switching\n");

  hal->mt_reset();  /* сбросим счетчик на 1 */
  hal->mt_enable();

  sched_yield();

  extern multiboot_info_t *__mbi;
  initrb = new ModuleFS(__mbi);

  printk("kernel: starting init\n");
  Tinterface *obj = initrb->access("init");
  string elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  hal->ProcMan->exec(elf_buf, "init");
  delete elf_buf;
  
  printk("--------------------------------------------------------------------------------" \
	 "All OK. Main kernel procedure done.\n"			\
	 "--------------------------------------------------------------------------------");

  while(1){
    sched_yield();
  }
}
