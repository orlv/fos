/*
  kernel/main/procman/procman.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <mm.h>
#include <procman.h>
#include <stdio.h>
#include <system.h>
#include <hal.h>
#include <string.h>

#include <drivers/fs/modulefs/modulefs.h>
#include <drivers/char/tty/tty.h>
#include <fs.h>

void start_sched();
void namer_srv();

void procman_srv()
{
  extern ModuleFS *initrb;
  Tinterface *object;
  Thread *thread;
  struct message *msg = new message;

  u32_t res;
  struct procman_message *pm = new procman_message;
  char *elf_buf;
  msg->tid = 0;

  char *kmesg;
  size_t len;
  
  printk("procman: registering /sys/procman\n");
  namer_add("/sys/procman");
  printk("procman: ready\n");
  
  while (1) {
    //asm("incb 0xb8000+154\n" "movb $0x2f,0xb8000+155 ");
    msg->recv_size = 256;
    msg->recv_buf = pm;
    receive(msg);
    //printf("ProcMan: cmd=%d, tid=%d\n", pm->cmd, msg->tid);
    
    switch(pm->cmd){
    case PROCMAN_CMD_EXEC:
      if((object = initrb->access(pm->arg.buf))){
	elf_buf = new char[object->info.size];
	object->read(0, elf_buf, object->info.size);
	hal->ProcMan->exec(elf_buf, pm->arg.buf);
	delete elf_buf;
	res = RES_SUCCESS;
      } else {
	res = RES_FAULT;
      }
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_KILL:
      if(hal->ProcMan->kill(pm->arg.pid)){
	res = RES_FAULT;
      } else {
	res = RES_SUCCESS;
      }
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_EXIT:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      if(hal->ProcMan->kill((tid_t)thread)){
	res = RES_FAULT;
      } else {
	res = RES_SUCCESS;
      }
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_MEM_ALLOC:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = (u32_t) thread->process->memory->mem_alloc(pm->arg.value);
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_MEM_MAP:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = (u32_t) thread->process->memory->mem_alloc_phys(pm->arg.val.a1, pm->arg.val.a2);
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_CREATE_THREAD:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      thread = thread->process->thread_create(pm->arg.value, FLAG_TSK_READY, kmalloc(PAGE_SIZE), thread->process->memory->mem_alloc(PAGE_SIZE));
      res = (u32_t) thread;
      hal->ProcMan->reg_thread(thread);
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_ATTACH:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = hal->interrupt_attach(thread, pm->arg.value);
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_DETACH:
      thread = hal->ProcMan->get_thread_by_tid(msg->tid);
      res = hal->interrupt_detach(thread, pm->arg.value);
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_DMESG:
      kmesg = new char[2000];
      extern TTY *stdout;
      len = stdout->read(0, kmesg, 2000);
      msg->send_buf = kmesg;
      msg->send_size = len;
      reply(msg);
      delete kmesg;
      break;
      
    default:
      res = RES_FAULT;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      msg->recv_size = 0;
      reply(msg);
    }

  }
}


TProcMan::TProcMan()
{
  hal->ProcMan = this;

  Thread *thread;
  void *stack;
  TProcess *process = new TProcess();

  process->name = "kernel";
  
  process->memory = new Memory(USER_MEM_BASE, USER_MEM_SIZE, MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS);
  process->memory->pagedir = hal->kmem->pagedir;

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create(0, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  threadlist = new List<Thread *>(thread);
  hal->gdt->load_tss(SEL_N(BASE_TSK_SEL), &thread->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);
  
  CurrentThread = thread;
  printk("kernel: multitasking ready (kernel tid=0x%X)\n", thread);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &start_sched, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  hal->gdt->load_tss(SEL_N(BASE_TSK_SEL) + 1, &thread->descr);
  printk("kernel: scheduler thread created (tid=0x%X)\n", thread);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &procman_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);
  printk("kernel: procman added to threads list (tid=0x%X)\n", thread);
  
  stack = kmalloc(STACK_SIZE);
  hal->tid_namer = (tid_t)process->thread_create((off_t) &namer_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(THREAD(hal->tid_namer));
  printk("procman: namer added to threads list (tid=0x%X)\n", hal->tid_namer);
}

u32_t TProcMan::exec(register void *image, const string name)
{
  TProcess *process = new TProcess();
  process->memory = new Memory(USER_MEM_BASE, USER_MEM_SIZE, MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS|MMU_PAGE_USER_ACCESSABLE);

  process->name = new char[strlen(name) + 1];
  strcpy(process->name, name);

  /* создаём каталог страниц процесса */
  process->memory->pagedir = (u32_t *) kmalloc(PAGE_SIZE);
  /* скопируем указатели на таблицы страниц ядра (страницы, расположенные ниже KERNEL_MEM_LIMIT) */
  for(u32_t i=0; i <= PAGE(KERNEL_MEM_LIMIT)/1024; i++){
    process->memory->pagedir[i] = hal->kmem->pagedir[i];
  }

  off_t eip;
  eip = process->LoadELF(image);
  Thread *thread = process->thread_create(eip, FLAG_TSK_READY, kmalloc(STACK_SIZE), process->memory->mem_alloc(STACK_SIZE));
  reg_thread(thread);
  return 0;
}

/* Добавляет поток в список */
void TProcMan::reg_thread(register Thread * thread)
{
  hal->mt_disable();
  threadlist->add_tail(thread);
  hal->mt_enable();
}

void TProcMan::unreg_thread(register List<Thread *> * thread)
{
  hal->mt_disable();
  delete thread->item;
  delete thread;
  hal->mt_enable();
}

res_t TProcMan::kill(register pid_t pid)
{
  hal->mt_disable();
  List<Thread *> *current = threadlist;
  /* то же, что get_process_by_pid() */
  do {
    if ((pid_t)current->item->process == pid){
      if(current->item->flags | FLAG_TSK_KERN){
	hal->mt_enable();
	return RES_FAULT;
      }

      current->item->flags &= ~FLAG_TSK_READY; /* снимем отметку выполнения с процесса */
      delete current->item->process;         /* уничтожим процесс */
      delete current;   /* удалим процесс из списка пройессов */
      hal->mt_enable();
      return RES_SUCCESS;
    }
    current = current->next;
  } while (current != threadlist);

  hal->mt_enable();
  return RES_FAULT;
}

/* возвращает указатель только в том случае, если поток существует */
Thread *TProcMan::get_thread_by_tid(register tid_t tid)
{
  hal->mt_disable();
  List<Thread *> *current = threadlist;

  do {
    if ((tid_t)current->item == tid){
      hal->mt_enable();
      return current->item;
    }

    current = current->next;
  } while (current != threadlist);

  hal->mt_enable();
  return 0;
}

void kill(pid_t pid)
{
  struct message *msg = new struct message;
  struct procman_message *pm = new procman_message;
  pm->cmd = PROCMAN_CMD_KILL;
  pm->arg.pid = pid;

  msg->send_buf = pm;
  msg->recv_buf = 0;
  msg->send_size = sizeof(struct procman_message);
  msg->recv_size = 0;
  msg->tid = 0;
  send(msg);
  delete msg;
  delete pm;
}
