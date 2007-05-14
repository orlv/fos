/*
  kernel/main/mtask/tasks.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <mm.h>
#include <tasks.h>
#include <stdio.h>
#include <system.h>
#include <hal.h>

void start_sched();

TProcMan::TProcMan()
{
  hal->ProcMan = this;
  extern u32_t *mpagedir;
  kPageDir = mpagedir;

  TProcess *rootproc = kprocess(0, FLAG_TSK_READY);

  hal->gdt->load_tss(BASE_TSK_SEL_N, &rootproc->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);

  TProcess *sched = kprocess((off_t) & start_sched, 0);
  hal->gdt->set_tss_descriptor((off_t) sched->tss, &sched->descr);
  hal->gdt->load_tss(BASE_TSK_SEL_N + 1, &sched->descr);
}

u32_t TProcMan::exec(register void *image)
{
  u32_t *PageDir = CreatePageDir();
  TProcess *Process = new TProcess(image, FLAG_TSK_READY, PageDir);
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
  delete(TProcess *) proc->data;
  delete proc;
}

res_t TProcMan::stop(register pid_t pid)
{
#warning TODO: FIX!!!!!!!!!!!!!!!
#if 0
  TListEntry *first = proclist->FirstEntry;
  TListEntry *iter = first;
  TProcess *proc;
  while (1) {
    proc = (TProcess *) iter->data;
    if (proc->pid == pid) {
      proc->flags = (proc->flags) | FLAG_TSK_TERM;
      break;
    }
    if (iter->next = first)
      return RES_FAULT;
    iter = iter->next;
  };
  return RES_SUCCESS;
#endif
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
