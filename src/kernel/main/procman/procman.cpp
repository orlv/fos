/*
  kernel/main/procman/procman.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/pager.h>
#include <fos/procman.h>
#include <fos/printk.h>
#include <fos/fos.h>
#include <fos/hal.h>
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

#define FOS_MAX_ELF_SECTION_SIZE 0x100000 

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

tid_t execute_module(char *pathname)
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
      result = hal->procman->exec(elf_image, pathname);
    delete elf_image;
  }
  return result;
}

tid_t execute(char *pathname)
{
  tid_t result = 0;
  
  printk("procman: executing %s\n", pathname);
  int fd = open(pathname, 0);

  if (fd != -1) {
    struct stat *statbuf = new struct stat;
    fstat(fd, statbuf);
    char *elf_image = new char[statbuf->st_size];
    read(fd, elf_image, statbuf->st_size);
    if(!check_ELF_image(elf_image, statbuf->st_size))
      result = hal->procman->exec(elf_image, pathname);
    delete statbuf;
    delete elf_image;
  }
  return result;
}

void procman_srv()
{
  hal->tid_namer = execute_module("namer");

  Thread *thread;
  char *kmesg;
  size_t len;

  struct message *msg = new message;
  msg->tid = 0;
  char *pathname = new char[MAX_PATH_LEN];

  execute_module("init");

  while (1) {
    //asm("incb 0xb8000+154\n" "movb $0x2f,0xb8000+155 ");
    msg->recv_size = MAX_PATH_LEN;
    msg->recv_buf = pathname;
    msg->tid = _MSG_SENDER_ANY;

    receive(msg);
    //printk("procman: a0=%d from [%s]\n", msg->a0, THREAD(msg->tid)->process->name);

    switch(msg->a0){
    case PROCMAN_CMD_EXEC:
      msg->a0 = execute(pathname);
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_KILL:
      if(hal->procman->kill(msg->a1, FLAG_TSK_TERM))
	msg->a0 = 1;
      else
	msg->a0 = 0;
      msg->send_size = 0;
      reply(msg);
      break;

      /* завершить все потоки в адресном пространстве */
    case PROCMAN_CMD_EXIT:
      thread = hal->procman->get_thread_by_tid(msg->tid);
      if(hal->procman->kill(TID(thread), FLAG_TSK_TERM))
	msg->a0 = 1;
      else
	msg->a0 = 0;
      msg->send_size = 0;
      reply(msg);
      break;

      /* завершить только данный поток */
    case PROCMAN_CMD_THREAD_EXIT:
      thread = hal->procman->get_thread_by_tid(msg->tid);
      if(hal->procman->kill(TID(thread), FLAG_TSK_EXIT_THREAD))
	msg->a0 = 1;
      else
	msg->a0 = 0;
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_CREATE_THREAD:
      thread = hal->procman->get_thread_by_tid(msg->tid);
      thread = thread->process->thread_create(msg->a1, FLAG_TSK_READY, kmalloc(PAGE_SIZE), thread->process->memory->mem_alloc(PAGE_SIZE));
      hal->procman->reg_thread(thread);
      msg->a0 = (u32_t) thread;
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_ATTACH:
      thread = hal->procman->get_thread_by_tid(msg->tid);
      msg->a0 = hal->interrupt_attach(thread, msg->a1);
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_INTERRUPT_DETACH:
      thread = hal->procman->get_thread_by_tid(msg->tid);
      msg->a0 = hal->interrupt_detach(thread, msg->a1);
      msg->send_size = 0;
      reply(msg);
      break;

    case PROCMAN_CMD_DMESG:
      //printk("procman: dmesg \n");
      kmesg = new char[2000];
      extern TTY *stdout;
      len = stdout->read(0, kmesg, 2000);
      msg->send_buf = kmesg;
      msg->send_size = len;
      reply(msg);
      delete kmesg;
      break;

    case FS_CMD_ACCESS:
      msg->a0 = 1;
      msg->send_size = 0;
      reply(msg);
      break;
      
    default:
      msg->a0 = RES_FAULT;
      msg->send_size = 0;
      reply(msg);
    }
  }
}

TProcMan::TProcMan()
{
  hal->procman = this;

  Thread *thread;
  void *stack;
  TProcess *process = new TProcess();

  process->name = "kernel";
  
  process->memory = hal->kmem;
  process->memory->pager->pagedir = hal->kmem->pager->pagedir;

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create(0, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  threadlist = new List<Thread *>(thread);
  hal->gdt->load_tss(SEL_N(BASE_TSK_SEL), &thread->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);
  
  current_thread = thread;
  printk("kernel: multitasking ready (kernel tid=0x%X)\n", thread);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &sched_srv, FLAG_TSK_KERN, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  hal->gdt->load_tss(SEL_N(BASE_TSK_SEL) + 1, &thread->descr);
  //printk("kernel: scheduler thread created (tid=0x%X)\n", thread);

  stack = kmalloc(STACK_SIZE);
  hal->tid_procman = TID(process->thread_create((off_t) &procman_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT));
  reg_thread(THREAD(hal->tid_procman));
  //printk("kernel: procman added to threads list (tid=0x%X)\n", hal->tid_procman);

  stack = kmalloc(STACK_SIZE);
  hal->tid_mm = TID(process->thread_create((off_t) &mm_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT));
  reg_thread(THREAD(hal->tid_mm));
  
  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &grub_modulefs_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);

  stack = kmalloc(STACK_SIZE);
  thread = process->thread_create((off_t) &vesafb_srv, FLAG_TSK_KERN | FLAG_TSK_READY, stack, stack, KERNEL_CODE_SEGMENT, KERNEL_DATA_SEGMENT);
  reg_thread(thread);
}

tid_t TProcMan::exec(register void *image, const string name)
{
  TProcess *process = new TProcess();

  process->memory = new Memory(USER_MEM_BASE, USER_MEM_SIZE);
  process->name = new char[strlen(name) + 1];
  strcpy(process->name, name);

  /* создаём каталог страниц процесса */
  process->memory->pager = new Pager(OFFSET(kmalloc(PAGE_SIZE)), MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS|MMU_PAGE_USER_ACCESSABLE);
  /* скопируем указатели на таблицы страниц ядра (страницы, расположенные ниже KERNEL_MEM_LIMIT) */
  for(u32_t i=0; i <= PAGE(KERNEL_MEM_LIMIT)/1024; i++){
    process->memory->pager->pagedir[i] = hal->kmem->pager->pagedir[i];
  }

  off_t eip = process->LoadELF(image);
  Thread *thread = process->thread_create(eip, FLAG_TSK_READY, kmalloc(STACK_SIZE), process->memory->mem_alloc(STACK_SIZE));
  reg_thread(thread);
  return TID(thread);
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

List<Thread *> *TProcMan::do_kill(List<Thread *> *thread)
{
  List<Thread *> *next;
  if(thread->item->flags & FLAG_TSK_TERM) {
    TProcess *process = thread->item->process;
    List<Thread *> *current = threadlist;
    do {
      next = current->next;
      if (current->item->process == process)
	unreg_thread(current);
      current = next;
    } while (current != threadlist);
    delete process;
  } else {
    next = thread->next;
    unreg_thread(thread);
  }
  
  return next;
}

res_t TProcMan::kill(register tid_t tid, u16_t flag)
{
  Thread *thread = get_thread_by_tid(tid);
  res_t result = RES_FAULT;

  if(thread) {
    hal->mt_disable();
    if(!(thread->flags & FLAG_TSK_KERN)) {

      thread->flags |= flag;      /* поставим флаг завершения */

      if(!(thread->flags & FLAG_TSK_SYSCALL))
	thread->flags &= ~FLAG_TSK_READY;  /* снимем флаг выполнения с процесса */

      result = RES_SUCCESS;
    }
    hal->mt_enable();
  }

  return result;
}

/* возвращает указатель только в том случае,
   если поток существует, и не помечен для удаления */
Thread *TProcMan::get_thread_by_tid(register tid_t tid)
{
  hal->mt_disable();
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

  hal->mt_enable();
  return result;
}
