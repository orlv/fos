/*
  kernel/main/procman/procman.cpp
  Copyright (C) 2005-2008 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/pager.h>
#include <fos/procman.h>
#include <fos/printk.h>
#include <fos/fos.h>
#include <string.h>

void grub_modulefs_srv();
void vesafb_srv();
void mm_srv();
void procman_srv();

TProcMan::TProcMan()
{
  system->procman = this;

  Thread *thread;
  void *stack;
  TProcess *process = new TProcess();

  task.pid = new tindex<TProcess>(128*64, 64);
  task.tid = new tindex<Thread>(128*64, 64);

  timer.threads = new List<Thread *>;
  
  process->name = "kernel";
  
  process->memory = system->kmem;
  process->memory->pager->pagedir = system->kmem->pager->pagedir;

  process->pid = task.pid->add(process);
  
  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create(0, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  task.wait = new List<Thread *>(thread);
  thread->me = task.active = new List<Thread *>(thread);

  set_initial_task(&thread->context);
  
  thread->tid = task.tid->add(thread);
  printk("kernel: multitasking ready (kernel tid=%d)\n", thread->tid);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &procman_srv, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);
  printk("kernel: procman added to threads list (tid=%d)\n", thread->tid);
  
  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &mm_srv, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);
  printk("kernel: mm_srv added to threads list (tid=%d)\n", thread->tid);
  
  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &grub_modulefs_srv, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &vesafb_srv, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);

  curr = task.active;
}

tid_t TProcMan::exec(register void *image, const char *name,
		     const char *args, size_t args_len,
		     const char *envp, size_t envp_len)
{
  TProcess *process = new TProcess();

  process->memory = new VMM(USER_MEM_BASE, USER_MEM_SIZE);
  process->name = strdup(name);

  /* создаём каталог страниц процесса */
  process->memory->pager = new Pager(OFFSET(kmalloc(PAGE_SIZE)), MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS|MMU_PAGE_USER_ACCESSABLE);
  /* скопируем указатели на таблицы страниц ядра (страницы, расположенные ниже KERNEL_MEM_LIMIT) */
  for(u32_t i=0; i <= PAGE(KERNEL_MEM_LIMIT)/1024; i++){
    process->memory->pager->pagedir[i] = system->kmem->pager->pagedir[i];
  }

  off_t eip = process->LoadELF(image);
  Thread *thread = process->thread_create(eip, 0 /*FLAG_TSK_READY*/, kmalloc(STACK_SIZE), process->memory->mmap(0, STACK_SIZE, 0, 0, 0));

  if(args) {
    char *args_buf = (char *) kmalloc(args_len);
    memcpy(args_buf, args, args_len);
    thread->context.tss->eax = (u32_t)process->memory->mmap(0, args_len, 0, (off_t)args_buf, system->kmem);
    thread->context.tss->ebx = args_len;
    kfree(args_buf, args_len);
  }

  if(envp) {
    char *envp_buf = (char *) kmalloc(envp_len);
    memcpy(envp_buf, envp, envp_len);
    thread->context.tss->ecx = (u32_t)process->memory->mmap(0, envp_len, 0, (off_t)envp_buf, system->kmem);
    thread->context.tss->edx = envp_len;
    kfree(envp_buf, envp_len);
  }

  process->pid = task.pid->add(process);
  reg_thread(thread);
  return thread->tid;
}

/* Добавляет поток в список */
tid_t TProcMan::reg_thread(register Thread * thread)
{
  system->preempt.disable();
  thread->me = task.active->add_tail(thread);
  thread->tid = task.tid->add(thread);
  system->preempt.enable();
  return thread->tid;
}

void TProcMan::unreg_thread(register List<Thread *> * thread)
{
  system->preempt.disable();
  task.tid->remove(thread->item->tid);
  delete thread->item;
  delete thread;
  system->preempt.enable();
}

List<Thread *> *TProcMan::do_kill(List<Thread *> *thread)
{
  List<Thread *> *next;
  system->preempt.disable();
  if(thread->item->flags & FLAG_TSK_TERM) {
    TProcess *process = thread->item->process;
    List<Thread *> *current, *n;// = task.active;
    list_for_each_safe(current, n, task.active){
      if (current->item->process == process)
      	unreg_thread(current);
    }
    delete process;
  } else {
    next = thread->next;
    unreg_thread(thread);
  }
  system->preempt.enable();
  //return next;
  return task.active;
}

res_t TProcMan::kill(register tid_t tid, u16_t flag)
{
  printk("procman: kill not implemented!\n");
#if 0
  Thread *thread = task.tid->get(tid);
  res_t result = RES_FAULT;

  if(thread) {
    system->preempt.disable();
    if(!(thread->flags & FLAG_TSK_KERN)) {

      thread->flags |= flag;      /* поставим флаг завершения */

      if(!(thread->flags & FLAG_TSK_SYSCALL))
	thread->flags &= ~FLAG_TSK_READY;  /* снимем флаг выполнения с процесса */

      result = RES_SUCCESS;
    }
    system->preempt.enable();
  }

  return result;
#endif
  return 0;
}
