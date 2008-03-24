/*
  kernel/main/procman/procman_srv.cpp
  Copyright (C) 2005-2008 Oleg Fedorov
*/

#include <fos/procman.h>
#include <fos/printk.h>
#include <fos/fos.h>
#include <fos/limits.h>
#include <string.h>
#include <fos/drivers/modulefs/modulefs.h>
#include <fos/drivers/tty/tty.h>
#include <sys/elf32.h>
#include <fcntl.h>
#include <unistd.h>
#include <fos/nsi.h>

#define FOS_MAX_ELF_SECTION_SIZE 0x2000000 

static char *buffer;

static int check_ELF_image(register void *image, register size_t image_size)
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

static tid_t execute_module(const char *pathname, const char *args, size_t args_len)
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

static tid_t execute(char *pathname, char *args, size_t args_len, char *envp, size_t envp_len)
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


static int procman_exec(struct message *msg)
{
  char *path = 0;
  size_t pathlen = 0;
  char *args = 0;
  char *envp = 0;
  if(((pathlen = strnlen(buffer, MAX_PATH_LEN) + 1) < MAX_PATH_LEN) && /* размер пути не превышает допустимый */
     ((pathlen + msg->arg[1] + msg->arg[2]) == msg->recv_size) && /* заявленный размер данных соответствует полученному */
     (msg->arg[1] <= ARG_MAX) && /* размер аргументов не больше максимально допустимого */
     (msg->arg[2] <= ENVP_MAX)) { /* ошибка: размер переменных окружения больше максимально допустимого */ 

    path = buffer;
    if(msg->arg[1])
      args = &buffer[pathlen];
    
    if(msg->arg[2])
      envp = &buffer[pathlen + msg->arg[1]];

    msg->arg[0] = execute(path, args, msg->arg[1], envp, msg->arg[2]);
  } else
    msg->arg[0] = 0;
      
  msg->send_size = 0;
  return 1;
}

static int procman_kill(struct message *msg)
{
  if(system->procman->kill(msg->arg[1], FLAG_TSK_TERM))
    msg->arg[0] = 1;
  else
    msg->arg[0] = 0;
  msg->send_size = 0;
  return 1;
}

/* завершить все потоки в адресном пространстве */
static int procman_exit(struct message *msg)
{
  Thread *thread = system->procman->task.tid->get(msg->tid);
  if(system->procman->kill(thread->tid, FLAG_TSK_TERM))
    msg->arg[0] = 1;
  else
    msg->arg[0] = 0;
  msg->send_size = 0;
  return 1;
}

/* завершить только данный поток */
static int procman_thread_exit(struct message *msg)
{
  Thread *thread = system->procman->task.tid->get(msg->tid);
  if(system->procman->kill(thread->tid, FLAG_TSK_EXIT_THREAD))
    msg->arg[0] = 1;
  else
    msg->arg[0] = 0;
  msg->send_size = 0;
  return 1;
}

static int procman_create_thread(struct message *msg)
{
  Thread *thread = system->procman->task.tid->get(msg->tid);
  thread = thread->process->thread_create(msg->arg[1], 0/*FLAG_TSK_READY*/, kmalloc(PAGE_SIZE), thread->process->memory->mmap(0, PAGE_SIZE, 0, 0, 0));
  msg->arg[0] = system->procman->reg_thread(thread);
  thread->context.tss->eax = msg->arg[2];
  msg->send_size = 0;
  return 1;
}

static int procman_interrupt_attach(struct message *msg)
{
  Thread *thread = system->procman->task.tid->get(msg->tid);
  msg->arg[0] = system->interrupt_attach(thread, msg->arg[1]);
  msg->send_size = 0;
  return 1;
}

static int procman_interrupt_detach(struct message *msg)
{
  Thread *thread = system->procman->task.tid->get(msg->tid);
  msg->arg[0] = system->interrupt_detach(thread, msg->arg[1]);
  return 1;
}

static int procman_dmesg(struct message *msg)
{
  char *kmesg = new char[2000];
  extern TTY *stdout;
  size_t len = stdout->read(0, kmesg, 2000);
  msg->send_buf = kmesg;
  msg->send_size = len;
  reply(msg);
  delete kmesg;
  return 0;
}

static int procman_access(struct message *msg)
{
  msg->arg[0] = 1;
  msg->send_size = 0;
  return 1;
}

void procman_srv()
{
  tid_t tid = execute_module("namer", "namer", 6);
  printk("procman: namer added to threads list (tid=%d)\n", tid);
  
  buffer = new char[MAX_PATH_LEN + ARG_MAX + ENVP_MAX + 1];
  buffer[MAX_PATH_LEN + ARG_MAX + ENVP_MAX] = 0;

  execute_module("init", "init", 5);

  nsi_t *interface = new nsi_t();
  interface->std.recv_buf = buffer;
  interface->std.recv_size = MAX_PATH_LEN + ARG_MAX + ENVP_MAX;

  interface->add(PROCMAN_CMD_EXEC, &procman_exec);
  interface->add(PROCMAN_CMD_KILL, &procman_kill);
  interface->add(PROCMAN_CMD_EXIT, &procman_exit);
  interface->add(PROCMAN_CMD_THREAD_EXIT, &procman_thread_exit);
  interface->add(PROCMAN_CMD_CREATE_THREAD, &procman_create_thread);
  interface->add(PROCMAN_CMD_INTERRUPT_ATTACH, &procman_interrupt_attach);
  interface->add(PROCMAN_CMD_INTERRUPT_DETACH, &procman_interrupt_detach);
  interface->add(PROCMAN_CMD_DMESG, &procman_dmesg);
  interface->add(FS_CMD_ACCESS, &procman_access);

  while (1) {
    interface->wait_message();
  };
}
