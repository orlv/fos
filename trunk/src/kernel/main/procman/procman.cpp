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

void sched_srv();
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

tid_t execute_module(char *pathname, char *args, size_t args_len)
{
  printk("procman: executing module %s\n", pathname);

  extern ModuleFS *initrb;
  tid_t result = 0;

  int fd = initrb->access(pathname);
  if(fd){
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
    struct stat *statbuf = new struct stat;
    fstat(fd, statbuf);
    char *elf_image = new char[statbuf->st_size];
    read(fd, elf_image, statbuf->st_size);
    if(!check_ELF_image(elf_image, statbuf->st_size))
      result = system->procman->exec(elf_image, pathname, args, args_len, envp, envp_len);
    delete statbuf;
    delete elf_image;
  }
  close(fd);
  return result;
}

void procman_srv()
{
  system->tid_namer = execute_module("namer", "namer", 6);

  Thread *thread;
  char *kmesg;
  size_t len;

  struct message *msg = new message;
  msg->tid = 0;
  char *data = new char[MAX_PATH_LEN + ARG_MAX + ENVP_MAX + 1];
  data[MAX_PATH_LEN + ARG_MAX + ENVP_MAX] = 0;

  execute_module("init", "init", 5);

  while (1) {
    //asm("incb 0xb8000+154\n" "movb $0x2f,0xb8000+155 ");
    msg->recv_size = MAX_PATH_LEN + ARG_MAX + ENVP_MAX;
    msg->recv_buf = data;
    msg->tid = _MSG_SENDER_ANY;

    receive(msg);
    //printk("procman: a0=%d from [%s]\n", msg->arg[0], THREAD(msg->tid)->process->name);

    switch(msg->arg[0]){
    case PROCMAN_CMD_EXEC: {
      char *path = 0;
      size_t pathlen = 0;
      char *args = 0;
      char *envp = 0;

      if(((pathlen = strnlen(data, MAX_PATH_LEN)) < MAX_PATH_LEN) && /* размер пути не превышает допустимый */
	 ((pathlen + msg->arg[1] + msg->arg[2]) == msg->recv_size) && /* заявленный размер данных соответствует полученному */
	 (msg->arg[1] <= ARG_MAX) && /* размер аргументов не больше максимально допустимого */
	 (msg->arg[2] > ENVP_MAX)) { /* ошибка: размер переменных окружения больше максимально допустимого */ 

	path = data;
	if(msg->arg[1])
	  args = &data[pathlen+1];

	if(msg->arg[2])
	  envp = &data[pathlen+1 + msg->arg[1]+1];

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
      thread = system->procman->get_thread_by_tid(msg->tid);
      if(system->procman->kill(TID(thread), FLAG_TSK_TERM))
	msg->arg[0] = 1;
      else
	msg->arg[0] = 0;
      msg->send_size = 0;
      reply(msg);
      break;

      /* завершить только данный поток */
    case PROCMAN_CMD_THREAD_EXIT:
      thread = system->procman->get_thread_by_tid(msg->tid);
      if(system->procman->kill(TID(thread), FLAG_TSK_EXIT_THREAD))
	msg->arg[0] = 1;
      else
	msg->arg[0] = 0;
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_CREATE_THREAD:
      thread = system->procman->get_thread_by_tid(msg->tid);
      thread = thread->process->thread_create(msg->arg[1], FLAG_TSK_READY, kmalloc(PAGE_SIZE), thread->process->memory->mmap(0, PAGE_SIZE, 0, 0, 0));
      system->procman->reg_thread(thread);
      msg->arg[0] = (u32_t) thread;
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_ATTACH:
      thread = system->procman->get_thread_by_tid(msg->tid);
      msg->arg[0] = system->interrupt_attach(thread, msg->arg[1]);
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_DETACH:
      thread = system->procman->get_thread_by_tid(msg->tid);
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

  process->name = "kernel";
  
  process->memory = system->kmem;
  process->memory->pager->pagedir = system->kmem->pager->pagedir;

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create(0, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  threadlist = new List<Thread *>(thread);
  system->gdt->load_tss(SEL_N(BASE_TSK_SEL), &thread->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);
  
  current_thread = thread;
  printk("kernel: multitasking ready (kernel tid=0x%X)\n", thread);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &sched_srv, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  system->gdt->load_tss(SEL_N(BASE_TSK_SEL) + 1, &thread->descr);
  //printk("kernel: scheduler thread created (tid=0x%X)\n", thread);

  stack = kmalloc(STACK_SIZE);
  system->tid_procman = TID(process->thread_create((off_t) &procman_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT));
  reg_thread(THREAD(system->tid_procman));
  //printk("kernel: procman added to threads list (tid=0x%X)\n", system->tid_procman);

  stack = kmalloc(STACK_SIZE);
  system->tid_mm = TID(process->thread_create((off_t) &mm_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT));
  reg_thread(THREAD(system->tid_mm));
  
  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &grub_modulefs_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &vesafb_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);
}

tid_t TProcMan::exec(register void *image, const string name,
		     const string args, size_t args_len,
		     const string envp, size_t envp_len)
{
  TProcess *process = new TProcess();

  process->memory = new VMM(USER_MEM_BASE, USER_MEM_SIZE);
  process->name = new char[strlen(name) + 1];
  strcpy(process->name, name);

  /* создаём каталог страниц процесса */
  process->memory->pager = new Pager(OFFSET(kmalloc(PAGE_SIZE)), MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS|MMU_PAGE_USER_ACCESSABLE);
  /* скопируем указатели на таблицы страниц ядра (страницы, расположенные ниже KERNEL_MEM_LIMIT) */
  for(u32_t i=0; i <= PAGE(KERNEL_MEM_LIMIT)/1024; i++){
    process->memory->pager->pagedir[i] = system->kmem->pager->pagedir[i];
  }

  off_t eip = process->LoadELF(image);
  Thread *thread = process->thread_create(eip, FLAG_TSK_READY, kmalloc(STACK_SIZE), process->memory->mmap(0, STACK_SIZE, 0, 0, 0));

  if(args) {
    char *args_buf = (char *) kmalloc(args_len);
    memcpy(args_buf, args, args_len);
    thread->tss->eax = (u32_t)process->memory->mmap(0, args_len, 0, (off_t)args_buf, system->kmem);
    kfree(args_buf, args_len);
  }

  if(envp) {
    char *envp_buf = (char *) kmalloc(envp_len);
    memcpy(envp_buf, envp, envp_len);
    thread->tss->ebx = (u32_t)process->memory->mmap(0, envp_len, 0, (off_t)envp_buf, system->kmem);
    kfree(envp_buf, envp_len);
  }
  
  reg_thread(thread);
  return TID(thread);
}

/* Добавляет поток в список */
void TProcMan::reg_thread(register Thread * thread)
{
  system->mt_disable();
  threadlist->add_tail(thread);
  system->mt_enable();
}

void TProcMan::unreg_thread(register List<Thread *> * thread)
{
  system->mt_disable();
  delete thread->item;
  delete thread;
  system->mt_enable();
}

List<Thread *> *TProcMan::do_kill(List<Thread *> *thread)
{
  List<Thread *> *next;
  system->mt_disable();
  if(thread->item->flags & FLAG_TSK_TERM) {
    TProcess *process = thread->item->process;
    List<Thread *> *current, *n;// = threadlist;
    list_for_each_safe(current, n, threadlist){
      if (current->item->process == process)
      	unreg_thread(current);
    }
    delete process;
  } else {
    next = thread->next;
    unreg_thread(thread);
  }
  system->mt_enable();
  //return next;
  return threadlist;
}

res_t TProcMan::kill(register tid_t tid, u16_t flag)
{
  Thread *thread = get_thread_by_tid(tid);
  res_t result = RES_FAULT;

  if(thread) {
    system->mt_disable();
    if(!(thread->flags & FLAG_TSK_KERN)) {

      thread->flags |= flag;      /* поставим флаг завершения */

      if(!(thread->flags & FLAG_TSK_SYSCALL))
	thread->flags &= ~FLAG_TSK_READY;  /* снимем флаг выполнения с процесса */

      result = RES_SUCCESS;
    }
    system->mt_enable();
  }

  return result;
}

/* возвращает указатель только в том случае,
   если поток существует, и не помечен для удаления */
Thread *TProcMan::get_thread_by_tid(register tid_t tid)
{
  system->mt_disable();
  List<Thread *> *current = threadlist;
  Thread *result = 0;

  /* проходим список потоков в поисках соответствия */
  do {
    if ((tid_t)current->item == tid) {
      if(current->item->flags & (FLAG_TSK_TERM | FLAG_TSK_EXIT_THREAD))
	result = 0;
      else
	result = current->item;
      
      break;
    }
    current = current->next;
  } while (current != threadlist);

  system->mt_enable();
  return result;
}
