/*
 * kernel/main/main.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <multiboot.h>
#include <drivers/char/tty/tty.h>
#include <mm.h>
#include <paging.h>
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

void halt();

TTY *stdout;

Keyboard *keyb;
TTime *SysTimer;

asmlinkage void keyboard_handler()
{
  keyb->handler();
};

static inline void EnableTimer()
{
  hal->outportb(0x21, hal->inportb(0x21) & 0xfe); /* Enable timer */
}

HAL *hal;

int printf(const char *fmt, ...)
{
  extern char printbuf[2000];
  int i = 0;
  va_list args;
  va_start(args, fmt);
  i = vsprintf(printbuf, fmt, args);
  va_end(args);

  printbuf[i] = 0;
  volatile struct message msg;
  msg.send_buf = msg.recv_buf = printbuf;
  msg.send_size = i + 1;
  msg.recv_size = 10;
  msg.pid = 2;
  syscall_send((struct message *)&msg);

  return i;
}

void procman(ModuleFS *bindir)
{
  Tinterface *object;
  struct message *msg = new message;
  char *reply = new char[3];
  struct procman_message *pm = new procman_message;
  char *elf_buf;
  msg->pid = 0;
  
  while (1) {
    asm("incb 0xb8000+154\n" "movb $0x5e,0xb8000+155 ");

    msg->recv_size = 256;
    msg->recv_buf = pm;
    syscall_receive(msg);

    printf("\nProcMan: cmd=%d, string=%s\n", pm->cmd, pm->buf);

    if((object = bindir->access((char *)pm->buf))){
      elf_buf = new char[object->info.size];
      object->read(0, elf_buf, object->info.size);
      hal->ProcMan->exec(elf_buf);
      delete elf_buf;
      strcpy(reply, "OK");
    } else {
      strcpy(reply, "ER");
    }

    msg->recv_size = 0;
    msg->send_size = 3;

    reply[2] = 0;
    msg->send_buf = reply;
    syscall_reply(msg);
  }
}

asmlinkage void init()
{
  extern const string version;
  extern u32_t build;
  extern const string compile_date, compile_time;

  extern multiboot_info_t *__mbi;

  init_memory();
  
  hal = new HAL(__mbi);
  
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
  tty1->SetTextColor(WHITE);

  stdout = tty1;

  *stdout << "FOS OS. Revision " << version << ". Build #" << build << " " << compile_date << " " << compile_time << "\n";

  printk("--------------------------------------------------------------------------------");

  hal->ProcMan = new TProcMan;
  SysTimer = new TTime;
  EnableTimer();

  extern multiboot_info_t *__mbi;
  ModuleFS *modules = new ModuleFS(__mbi);
  Tinterface *obj;

  obj = modules->access("tty");
  string elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  hal->ProcMan->exec(elf_buf);
  delete elf_buf;
    
  obj = modules->access("fs");
  elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  hal->ProcMan->exec(elf_buf);
  delete elf_buf;
 
  printf("\n--------------------------------------------------------------------------------" \
	 "All OK. Init done.\n" \
	 "You can get new version at http://fos.osdev.ru/" \
	 "\n--------------------------------------------------------------------------------");

  procman(modules);
  hal->panic("Process Manager done");
}
