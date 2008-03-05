/*
  kernel/main/procman/procman.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/pager.h>
#include <fos/procman.h>
#include <fos/printk.h>
#include <fos/fos.h>
#include <fos/limits.h>
#include <string.h>

#include <fos/drivers/fs/modulefs/modulefs.h>
#include <fos/drivers/char/tty/tty.h>
#include <fos/fs.h>
#include <sys/elf32.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

void grub_modulefs_srv();
void vesafb_srv();
void mm_srv();

#define FOS_MAX_ELF_SECTION_SIZE 0x2000000 

int check_ELF_image(register void *image, register size_t image_size)
{
  Elf32_Phdr *p;
  Elf32_Ehdr *h = (Elf32_Ehdr *) image; /* образ ELF */

  if((image_size < sizeof(Elf32_Ehdr)) ||
     ((u32_t) h->e_phoff < sizeof(Elf32_Ehdr)) ||
     (image_size < (u32_t) h->e_phoff + sizeof(Elf32_Phdr)*h->e_phnum)) {
    printk("Invalid ELF headers!\n");
    return 1;
  }
 
  Elf32_Phdr *ph = (Elf32_Phdr *) ((u32_t) h + (u32_t) h->e_phoff);

  for (p = ph; p < ph + h->e_phnum; p++){
    if (p->p_type == ELF32_TYPE_LOAD && p->p_memsz) {
      if ((p->p_filesz > p->p_memsz) ||
	  (p->p_memsz > FOS_MAX_ELF_SECTION_SIZE)) {
	printk("Invalid ELF section! Section file_size=0x%X, mem_size=0x%X, FOS_MAX_ELF_SECTION_SIZE=0x%X\n", p->p_filesz, p->p_memsz, FOS_MAX_ELF_SECTION_SIZE);
	return 2;
      }

      if(p->p_filesz && (image_size < p->p_offset + p->p_filesz)) {
	printk("Invalid ELF section: bigger than image size\n");
	return 3;
      }
      
      if((p->p_vaddr & 0xfffff000) < USER_MEM_BASE) {
	printk("Can't map ELF section to 0x%X, USER_MEM_BASE=0x%X\n", p->p_vaddr, USER_MEM_BASE);
	return 4;
      }
    }
  }

  return 0;
}

tid_t execute_module(const char *pathname, const char *args, size_t args_len)
{
  printk("procman: executing module %s\n", pathname);

  extern ModuleFS *initrb;
  tid_t result = 0;

  int fd = initrb->access(pathname);

  if(fd >= 0){
    size_t size = initrb->size(fd);
    char *elf_image = new char[size];
    initrb->read(fd, 0, elf_image, size);
    if(!check_ELF_image(elf_image, size))
      result = system->procman->exec(elf_image, pathname, args, args_len, 0, 0);
    delete elf_image;
  }
  return result;
}

tid_t execute(char *pathname, char *args, size_t args_len, char *envp, size_t envp_len)
{
  tid_t result = 0;
#if 0
  if(args)
    printk("procman: executing %s with args [%s]\n", pathname, args);
  else
    printk("procman: executing %s with no args\n", pathname);
#endif
  int fd = open(pathname, 0);

  if (fd != -1) {
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char *elf_image = new char[size];
    read(fd, elf_image, size);
    if(!check_ELF_image(elf_image, size))
      result = system->procman->exec(elf_image, pathname, args, args_len, envp, envp_len);
    delete elf_image;
  }
  close(fd);
  return result;
}

void procman_srv()
{
  Thread *thread;
  char *kmesg;
  size_t len;
  tid_t tid = execute_module("namer", "namer", 6);
  printk("procman: namer added to threads list (tid=%d)\n", tid);
  
  struct message *msg = new message;
  char *data = new char[MAX_PATH_LEN + ARG_MAX + ENVP_MAX + 1];
  data[MAX_PATH_LEN + ARG_MAX + ENVP_MAX] = 0;

  execute_module("init", "init", 5);

  while (1) {
  //  asm("incb 0xb8000+154\n" "movb $0x2f,0xb8000+155 ");
    msg->recv_size = MAX_PATH_LEN + ARG_MAX + ENVP_MAX;
    msg->recv_buf = data;
    msg->tid = 0;
    msg->flags = 0;

    receive(msg);
    //printk("procman: a0=%d from [%s]\n", msg->arg[0], THREAD(msg->tid)->process->name);

    switch(msg->arg[0]){
    case PROCMAN_CMD_EXEC: {
      char *path = 0;
      size_t pathlen = 0;
      char *args = 0;
      char *envp = 0;
      if(((pathlen = strnlen(data, MAX_PATH_LEN) + 1) < MAX_PATH_LEN) && /* размер пути не превышает допустимый */
	 ((pathlen + msg->arg[1] + msg->arg[2]) == msg->recv_size) && /* заявленный размер данных соответствует полученному */
	 (msg->arg[1] <= ARG_MAX) && /* размер аргументов не больше максимально допустимого */
	 (msg->arg[2] <= ENVP_MAX)) { /* ошибка: размер переменных окружения больше максимально допустимого */ 

	path = data;
	if(msg->arg[1])
	  args = &data[pathlen];

	if(msg->arg[2]) {
	  envp = &data[pathlen + msg->arg[1]];
	}

	msg->arg[0] = execute(path, args, msg->arg[1], envp, msg->arg[2]);
      } else {
	msg->arg[0] = 0;
      }
      
      msg->send_size = 0;
      reply(msg);
      break;
    }
      
    case PROCMAN_CMD_KILL:
      if(system->procman->kill(msg->arg[1], FLAG_TSK_TERM))
	msg->arg[0] = 1;
      else
	msg->arg[0] = 0;
      msg->send_size = 0;
      reply(msg);
      break;

      /* завершить все потоки в адресном пространстве */
    case PROCMAN_CMD_EXIT:
      thread = system->procman->task.tid->get(msg->tid);
      if(system->procman->kill(thread->tid, FLAG_TSK_TERM))
	msg->arg[0] = 1;
      else
	msg->arg[0] = 0;
      msg->send_size = 0;
      reply(msg);
      break;

      /* завершить только данный поток */
    case PROCMAN_CMD_THREAD_EXIT:
      thread = system->procman->task.tid->get(msg->tid);
      if(system->procman->kill(thread->tid, FLAG_TSK_EXIT_THREAD))
	msg->arg[0] = 1;
      else
	msg->arg[0] = 0;
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_CREATE_THREAD:
      thread = system->procman->task.tid->get(msg->tid);
      //thread = system->procman->get_thread_by_tid(msg->tid);
      thread = thread->process->thread_create(msg->arg[1], 0/*FLAG_TSK_READY*/, kmalloc(PAGE_SIZE), thread->process->memory->mmap(0, PAGE_SIZE, 0, 0, 0));
      msg->arg[0] = system->procman->reg_thread(thread);
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_ATTACH:
      thread = system->procman->task.tid->get(msg->tid);
      //thread = system->procman->get_thread_by_tid(msg->tid);
      msg->arg[0] = system->interrupt_attach(thread, msg->arg[1]);
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_DETACH:
      thread = system->procman->task.tid->get(msg->tid);
      //thread = system->procman->get_thread_by_tid(msg->tid);
      msg->arg[0] = system->interrupt_detach(thread, msg->arg[1]);
      msg->send_size = 0;
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

    case FS_CMD_ACCESS:
      msg->arg[0] = 1;
      msg->send_size = 0;
      reply(msg);
      break;
      
    default:
      msg->arg[0] = RES_FAULT;
      msg->send_size = 0;
      reply(msg);
    }
  }
}

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
  system->gdt->load_tss(SEL_N(BASE_TSK_SEL), &thread->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);
  
  //current_thread = thread;
  thread->tid = task.tid->add(thread);
  printk("kernel: multitasking ready (kernel tid=%d)\n", thread->tid);

  /*  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &sched_srv, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  system->gdt->load_tss(SEL_N(BASE_TSK_SEL) + 1, &thread->descr);
  thread->tid = task.tid->add(thread);
  printk("kernel: scheduler thread created (tid=%d)\n", thread->tid);*/

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
    thread->tss->eax = (u32_t)process->memory->mmap(0, args_len, 0, (off_t)args_buf, system->kmem);
    thread->tss->ebx = args_len;
    kfree(args_buf, args_len);
  }

  if(envp) {
    char *envp_buf = (char *) kmalloc(envp_len);
    memcpy(envp_buf, envp, envp_len);
    thread->tss->ecx = (u32_t)process->memory->mmap(0, envp_len, 0, (off_t)envp_buf, system->kmem);
    thread->tss->edx = envp_len;
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
