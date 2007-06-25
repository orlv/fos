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

TTY *stdout;
TTime *SysTimer;
HAL *hal;

static inline void EnableTimer()
{
  hal->pic->unmask(0);
}

tid_t resolve(char *name)
{
  while(!hal->namer);
  volatile struct message msg;
  u32_t res;
  union fs_message m;
  msg.recv_size = sizeof(res);
  msg.recv_buf = &res;
  msg.send_size = sizeof(fs_message);
  m.data3.cmd = NAMER_CMD_RESOLVE;
  strcpy((char *)m.data3.buf, name);
  msg.send_buf = (char *)&m;
  msg.tid = 0;
  send((message *)&msg);
  return res;
}

void namer_add(string name)
{
  while(!hal->tid_namer);
  struct message *msg = new struct message;
  u32_t res;
  union fs_message *m = new fs_message;
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(union fs_message);
  msg->send_buf = m;
  strcpy(m->data3.buf,  name);
  m->data3.cmd = NAMER_CMD_ADD;
  msg->tid = 0;
  send(msg);
}

void procman(ModuleFS *bindir)
{
  Tinterface *object;
  Thread *thread;
  struct message *msg = new message;

  u32_t res;
  struct procman_message *pm = new procman_message;
  char *elf_buf;
  msg->tid = 0;

  while (1) {
    asm("incb 0xb8000+154\n" "movb $0x2f,0xb8000+155 ");

    msg->recv_size = 256;
    msg->recv_buf = pm;
    receive(msg);
    printf("ProcMan: cmd=%d, tid=%d\n", pm->cmd, msg->tid);
    
    switch(pm->cmd){
    case PROCMAN_CMD_EXEC:
      if((object = bindir->access(pm->arg.buf))){
	elf_buf = new char[object->info.size];
	object->read(0, elf_buf, object->info.size);
	hal->ProcMan->exec(elf_buf, pm->arg.buf);
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
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      if(hal->ProcMan->kill((tid_t)thread)){
	res = RES_FAULT;
      } else {
	res = RES_SUCCESS;
      }

      break;

    case PROCMAN_CMD_MEM_ALLOC:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = (u32_t) thread->process->memory->mem_alloc(pm->arg.value);
      break;

    case PROCMAN_CMD_MEM_MAP:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = (u32_t) thread->process->memory->mem_alloc_phys(pm->arg.val.a1, pm->arg.val.a2);
      break;

    case PROCMAN_CMD_CREATE_THREAD:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      thread = thread->process->thread_create(pm->arg.value, FLAG_TSK_READY, kmalloc(PAGE_SIZE), thread->process->memory->mem_alloc(PAGE_SIZE));
      res = (u32_t) thread;
      hal->ProcMan->reg_thread(thread);
      break;

    case PROCMAN_CMD_INTERRUPT_ATTACH:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = hal->interrupt_attach(thread, pm->arg.value);
      break;

    case PROCMAN_CMD_INTERRUPT_DETACH:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = hal->interrupt_detach(thread, pm->arg.value);
      break;
      
    default:
      res = RES_FAULT;
    }
	
    msg->recv_size = 0;
    msg->send_size = sizeof(res);

    msg->send_buf = &res;
    reply(msg);
  }
}

void out_banner()
{
  extern const string version;
  extern u32_t build;
  extern const string compile_date, compile_time;

  printk("FOS OS. Revision %s. Build #%s %s %s \n"			\
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
  tty1->SetTextColor(WHITE);

  stdout = tty1;

  out_banner();

  hal->ProcMan = new TProcMan;
  SysTimer = new TTime;

  EnableTimer();
  hal->mt_enable();

  namer_add("/sys/procman");

  tid_t pro = resolve("/sys/procman");
  printk("pro=0x%X\n", pro);
  
  extern multiboot_info_t *__mbi;
  ModuleFS *modules = new ModuleFS(__mbi);
  Tinterface *obj;

  obj = modules->access("init");
  string elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  hal->ProcMan->exec(elf_buf, "init");
  delete elf_buf;

  printk("\n--------------------------------------------------------------------------------" \
	 "All OK. Kernel init done.\n"					\
	 "You can get new version at http://fos.osdev.ru/"		\
	 "\n--------------------------------------------------------------------------------");

  procman(modules);
}
