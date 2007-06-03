/*
  kernel/main/procman/procman.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <mm.h>
#include <procman.h>
#include <stdio.h>
#include <system.h>
#include <hal.h>

void start_sched();
void namer_srv();

TProcMan::TProcMan()
{
  hal->ProcMan = this;
  extern u32_t *mpagedir;
  kPageDir = mpagedir;

  Thread *thread;
  TProcess *process;
  process = kprocess(0, FLAG_TSK_READY);
  thread = (Thread *) process->threads->data;
  CurrentThread = thread;
  hal->gdt->load_tss(BASE_TSK_SEL_N, &thread->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);

  process = kprocess((off_t) & start_sched, 0);
  hal->gdt->load_tss(BASE_TSK_SEL_N + 1, &((Thread *) process->threads->data)->descr);

  process = kprocess((off_t) &namer_srv, FLAG_TSK_READY);
  hal->tid_namer = (tid_t) process->threads->data;
}

u32_t TProcMan::exec(register void *image)
{
  TProcess *process = new TProcess(FLAG_TSK_READY, image, 0);
  add((Thread *)process->threads->data);
  return 0;
}

/* Добавляет поток в список */
void TProcMan::add(register Thread * thread)
{
  proclist->add_tail(thread);
}

void TProcMan::del(register List * proc)
{
  delete (Thread *)proc->data;
  delete proc;
}

res_t TProcMan::kill(register pid_t pid)
{
  Thread *thread;
  List *current = proclist;
  /* то же, что get_process_by_pid() */
  do {
    thread = (Thread *) current->data;
    if ((pid_t)thread->process == pid){
      if(thread->flags | FLAG_TSK_KERN)
	return RES_FAULT;

      thread->flags &= ~FLAG_TSK_READY; /* снимем отметку выполнения с процесса */
      delete current;   /* удалим процесс из списка пройессов */
      delete thread->process;         /* уничтожим процесс */
      return RES_SUCCESS;
    }
    current = current->next;
  } while (current != proclist);

  return RES_FAULT;
}

/* возвращает указатель только в том случае, если поток существует */
Thread *TProcMan::get_thread_by_tid(register tid_t tid)
{
  List *current = proclist;
  Thread *thread;

  do {
    thread = (Thread *) current->data;
    if ((tid_t)thread == tid){
      return thread;
    }

    current = current->next;
  } while (current != proclist);

  return 0;
}

/*
  Создаёт процесс в адресном пространстве ядра.
  Полезен для создания серверов, удобно разделяющих структуры данных с другими процессами режима ядра
*/
TProcess *TProcMan::kprocess(register off_t eip, register u16_t flags)
{
  TProcess *process = new TProcess(flags | FLAG_TSK_KERN, (void *) eip, kPageDir);

  /* Зарегистрируем процесс */
  if(proclist)
    proclist->add_tail(process->threads->data);
  else
    proclist = new List(process->threads->data);

  return process;
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
