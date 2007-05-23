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
void namer();

TProcMan::TProcMan()
{
  hal->ProcMan = this;
  extern u32_t *mpagedir;
  kPageDir = mpagedir;

  TProcess *rootproc = kprocess(0, FLAG_TSK_READY);
  CurrentProcess = rootproc;
    
  hal->gdt->load_tss(BASE_TSK_SEL_N, &rootproc->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);

  TProcess *sched = kprocess((off_t) & start_sched, 0);
  hal->gdt->set_tss_descriptor((off_t) sched->tss, &sched->descr);
  hal->gdt->load_tss(BASE_TSK_SEL_N + 1, &sched->descr);

  TProcess *fs = kprocess((off_t) &namer, FLAG_TSK_READY);
  hal->gdt->set_tss_descriptor((off_t) fs->tss, &fs->descr);
}

u32_t TProcMan::exec(register void *image)
{
  TProcess *Process = new TProcess(FLAG_TSK_READY, image, 0);
  add(Process);
  return 0;
}

/* Добавляет процесс в список процессов */
void TProcMan::add(register TProcess * process)
{
  proclist->add_tail(process);
}

void TProcMan::del(register List * proc)
{
  delete (TProcess *)proc->data;
  delete proc;
}

res_t TProcMan::kill(register pid_t pid)
{
  TProcess *p;
  List *current = proclist;
  /* то же, что get_process_by_pid() */
  do {
    p = (TProcess *) current->data;
    if (p->pid == pid){
      if(p->flags | FLAG_TSK_KERN)
	return RES_FAULT;

      p->flags &= ~FLAG_TSK_READY; /* снимем отметку выполнения с процесса */
      delete current;   /* удалим процесс из списка пройессов */
      delete p;         /* уничтожим процесс */
      return RES_SUCCESS;
    }
    current = current->next;
  } while (current != proclist);

  return RES_FAULT;
}

TProcess *TProcMan::get_process_by_pid(register u32_t pid)
{
  List *current = proclist;

  do {
    if (((TProcess *) current->data)->pid == pid)
      return (TProcess *) current->data;

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
  TProcess *proc = new TProcess(flags | FLAG_TSK_KERN, 0, kPageDir);

  proc->set_stack_pl0();
  proc->kprocess_set_tss(eip, (u32_t *)load_cr3());

  /* Зарегистрируем процесс */
  if(proclist)
    proclist->add_tail(proc);
  else
    proclist = new List(proc);

  return proc;
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
  msg->pid = 0;
  send(msg);
  delete msg;
  delete pm;
}
