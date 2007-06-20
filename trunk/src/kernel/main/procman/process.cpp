/*
  kernel/main/procman/process.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <mm.h>
#include <paging.h>
#include <procman.h>
#include <process.h>
#include <stdio.h>
#include <system.h>
#include <hal.h>
#include <elf32.h>
#include <string.h>

//extern volatile u32_t top_pid;

TProcess::TProcess()
{
  //u32_t eip;
  //  if(flags & FLAG_TSK_KERN){
  //  this->mem_init(KERNEL_MEM_BASE, KERNEL_MEM_SIZE);
  //} else {
  //this->mem_init(PROCESS_MEM_BASE, PROCESS_MEM_SIZE);
  //}
  
  //if(flags & FLAG_TSK_KERN){ /* kernel process */
  //this->PageDir = kPageDir;
  //eip = (off_t) image;
  //} else {                   /* user process   */
  //this->PageDir = CreatePageDir();
  //eip = LoadELF(image);
  //}

  //thread_create(eip, flags);
}

TProcess::~TProcess()
{
  hal->panic("Terminating process is not implemented");
#if 0
  List *curr, *n;

  /* освободим выделенную память */
  task_mem_block_t *c;
  list_for_each_safe(curr, n, UsedMem){
    c = (task_mem_block_t *) curr->data;
    //    printk("0x%X 0x%X 0x%X \n", c->vptr, c->pptr, c->size);
    kfree((void *)c->pptr);
    delete(task_mem_block_t *) curr->data;
    delete curr;
  }

  c = (task_mem_block_t *) UsedMem->data;
  //printk("0x%X 0x%X 0x%X \n", c->vptr, c->pptr, c->size);
  kfree((void *)c->pptr);
  delete(task_mem_block_t *) curr->data;
  delete curr;

  /* удалим таблицы свободных блоков */
  list_for_each_safe(curr, n, FreeMem){
    delete (task_mem_block_t *)curr->data;
    delete curr;
  }
  delete (task_mem_block_t *)curr->data;
  delete curr;

  /* удаляем все потоки */
  list_for_each_safe(curr, n, threads){
    delete (Thread *)curr->data;
    delete curr;
  }
  delete (Thread *)curr->data;
  delete curr;

  
  u32_t i;
  for(i = USER_MEM_START/1024; i < 1024; i++){
    if(PageDir[i])
      kfree((void *)(((u32_t)PageDir[i]) & 0xfffff000));
  }
  kfree((void *)(((u32_t)PageDir) & 0xfffff000));
#endif
}

u32_t TProcess::LoadELF(register void *image)
{
  u32_t *object;
  Elf32_Phdr *p;
  Elf32_Ehdr *h = (Elf32_Ehdr *) image; /* образ ELF */
  Elf32_Phdr *ph = (Elf32_Phdr *) ((u32_t) h + (u32_t) h->e_phoff);
  
#if 0
  /* на всякий случай оставлю этот код (вывод информации об Section Headers) */
  Elf32_Shdr *sh = (Elf32_Shdr *) ((u32_t)h + (u32_t)h->e_shoff);
  for(i=0; i < h->e_shnum-1; i++)
    {
      if(sh[i].sh_flags && SHF_ALLOC)
        {
	  printk("load addr=0x%X, file offset=0x%X, size=0x%X flags=0x%X\n",
	  	 sh[i].sh_addr, sh[i].sh_offset, sh[i].sh_size, sh[i].sh_flags);
        }
    }
#endif

  /*
    Если секция обозначена как загружаемая - выделим для неё память,
    скопируем из файла данные и смонтируем в адресное пространство процесса
   */
  for (p = ph; p < ph + h->e_phnum; p++){
    if (p->p_type == ELF32_TYPE_LOAD && p->p_memsz) {
      //printk("flags=%d, addr=0x%X, type=%d, fileoffs=0x%X, filesz=0x%X, memsz=0x%X \n",
      //p->p_flags, p->p_vaddr, p->p_type, p->p_offset, p->p_filesz, p->p_memsz);
      
      if (p->p_filesz > p->p_memsz)
	hal->panic("Invalid section!");

      /*
	Выделим память под секцию
	Учтём, что начало секции может быть не выровнено по началу страницы
      */
      object = (u32_t *) kmalloc(p->p_memsz + (p->p_vaddr % PAGE_SIZE)); 
      //printk("sz=0x%X \n",p->p_filesz/sizeof(u32_t));

      if (p->p_filesz > 0) {
	memcpy((u32_t *) ((u32_t)object + (p->p_vaddr % PAGE_SIZE)), (u32_t *) ((u32_t) image + p->p_offset), p->p_filesz);
      }

      /* Монтируем секцию в адресное пространство процесса */
      memory->mmap(object, (u32_t *) (p->p_vaddr & 0xfffff000), p->p_memsz + (p->p_vaddr % PAGE_SIZE));
    }
  }

  /* Возвращаем указатель на точку входа */
  return h->e_entry;
}


Thread *TProcess::thread_create(off_t eip, u16_t flags)
{
  Thread *thread = new Thread(this, eip, flags);
  
  if(!threads){
    threads = new List(thread);
  } else {
    threads->add_tail(thread);
  }
  return thread;
}

Thread::Thread(TProcess *process, off_t eip, u16_t flags)
{
  struct message *_msg = new(struct message);
  new_msg = new List(_msg);   /* пустое сообщение */
  recvd_msg = new List(_msg); /* пустое сообщение */

  this->process = process;
  
  set_tss(eip);

  this->flags = flags;
}

Thread::~Thread()
{
  List *curr, *n;
  /* удалим все сообщения */
#warning вернуть ошибку отправителям
  list_for_each_safe(curr, n, new_msg){
    delete (message *)curr->data;
    delete curr;
  }

  delete (message *)new_msg->data;
  delete new_msg;

  kfree((void *)stack_pl0);
  delete tss;
}

void Thread::set_tss(register off_t eip)
{
  if (!(tss = (struct TSS *)kmalloc(sizeof(struct TSS))))
    hal->panic("No memory left to create tss.");

  tss->cr3 = (u32_t)process->memory->pagedir;
  tss->eip = eip;

  tss->eflags = 0x00000202;

  stack_pl0 = (u32_t)kmalloc(STACK_SIZE);
  tss->esp0 = stack_pl0 + STACK_SIZE - 1;

  if(!(flags & FLAG_TSK_KERN)){ /* user process */
    tss->esp = tss->ebp = (u32_t)process->memory->mem_alloc(STACK_SIZE) + STACK_SIZE - 1;
  
    tss->cs = USER_CODE;
    tss->es = USER_DATA;
    tss->ss = USER_DATA;
    tss->ds = USER_DATA;
    tss->ss0 = KERNEL_DATA;
  } else {
    tss->esp = tss->ebp = stack_pl0 + STACK_SIZE - 1;

    tss->cs = KERNEL_CODE;
    tss->es = KERNEL_DATA;
    tss->ss = KERNEL_DATA;
    tss->ds = KERNEL_DATA;
  }
  
  tss->IOPB = 0xffffffff;

  /* Установим TSS */
  hal->gdt->set_tss_descriptor((off_t) tss, &descr);
}

void Thread::run()
{
  hal->gdt->load_tss(BASE_TSK_SEL_N, &descr);
  asm("ljmp $0x38, $0");
}
