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
#include <drivers/char/keyboard/keyboard.h>

#include <fs.h>

void halt();

TTY *stdout;

Keyboard *keyb;
TTime *SysTimer;

static inline void EnableTimer()
{
  hal->outportb(0x21, hal->inportb(0x21) & 0xfe); /* Enable timer */
}

HAL *hal;

void namer_add(string name)
{
  struct message *msg = new struct message;
  u32_t res;
  struct fs_message *m = new fs_message;
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(struct fs_message);
  msg->send_buf = m;
  strcpy(m->buf,  name);
  m->cmd = NAMER_CMD_ADD;
  msg->tid = (tid_t)hal->namer;
  send(msg);
}

void procman(ModuleFS *bindir)
{
#if 1
  Tinterface *object;
  Thread *thread;
  struct message *msg = new message;

  u32_t res;
  struct procman_message *pm = new procman_message;
  char *elf_buf;
  msg->tid = 0;

  while (1) {
    asm("incb 0xb8000+154\n" "movb $0x2f,0xb8000+155 ");
    //asm("incb 0xb8000+154\n" "movb $0x5e,0xb8000+155 ");

    msg->recv_size = 256;
    msg->recv_buf = pm;
    receive(msg);
    //printk("\nProcMan: cmd=%d, pid=%d\n", pm->cmd, msg->pid);

    switch(pm->cmd){
    case PROCMAN_CMD_EXEC:
      if((object = bindir->access(pm->arg.buf))){
	elf_buf = new char[object->info.size];
	object->read(0, elf_buf, object->info.size);
	hal->ProcMan->exec(elf_buf);
	delete elf_buf;
	res = RES_SUCCESS;
      } else {
	res = RES_FAULT;
      }
      break;

    case PROCMAN_CMD_KILL:
      if(hal->ProcMan->kill(pm->arg.pid)){
	res = RES_FAULT;
      } else {
	res = RES_SUCCESS;
      }

      break;

    case PROCMAN_CMD_EXIT:
      if(hal->ProcMan->kill(msg->tid)){
	res = RES_FAULT;
      } else {
	res = RES_SUCCESS;
      }

      break;

    case PROCMAN_CMD_MEM_ALLOC:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = (u32_t) thread->process->mem_alloc(pm->arg.value);
      //printk("\nProcMan: ptr=0x%X\n", reply);
      break;

    case PROCMAN_CMD_MEM_MAP:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      //printk("\nProcMan: a1=0x%X, a2=0x%X\n", pm->arg.val.a1, pm->arg.val.a2);
      res = (u32_t) thread->process->mem_alloc(pm->arg.val.a1, pm->arg.val.a2);
      //printk("\nProcMan: ptr=0x%X\n", reply);
      break;
      
    default:
      res = RES_FAULT;
    }
	
    msg->recv_size = 0;
    msg->send_size = sizeof(res);

    msg->send_buf = &res;
    reply(msg);
  }
#endif
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
#if 0
  struct message *msg = new struct message;
  u32_t res;
  struct fs_message *m = new fs_message;
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(struct fs_message);
  msg->send_buf = m;
  strcpy(m->buf, "/sys/namer");
  m->cmd = NAMER_CMD_RESOLVE;
  msg->tid = 0;
  send(msg);

  printk("res=0x%X\n", res);
#endif

  namer_add("/sys/procman");
  
  keyb = new Keyboard;
  
  extern multiboot_info_t *__mbi;
  ModuleFS *modules = new ModuleFS(__mbi);
  Tinterface *obj;
  
  obj = modules->access("init");
  string elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  hal->ProcMan->exec(elf_buf);
  delete elf_buf;
  
  printk("\n--------------------------------------------------------------------------------" \
	 "All OK. Init done.\n" \
	 "You can get new version at http://fos.osdev.ru/" \
	 "\n--------------------------------------------------------------------------------");
  
  procman(modules);
}
