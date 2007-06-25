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
#include <paging.h>

void start_sched();
void namer_srv();

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

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &start_sched, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);

  hal->gdt->load_tss(SEL_N(BASE_TSK_SEL) + 1, &thread->descr);

  stack = kmalloc(STACK_SIZE);

  hal->tid_namer = (tid_t)process->thread_create((off_t) &namer_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(THREAD(hal->tid_namer));  
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
