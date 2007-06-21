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


Thread *TProcess::thread_create(off_t eip,
				u16_t flags,
				void * kernel_stack,
				void * user_stack,
				u16_t code_segment,
				u16_t data_segment)

{
  Thread *thread = new Thread(this, eip, flags, kernel_stack, user_stack , code_segment, data_segment);
  
  if(!threads){
    threads = new List<Thread *>(thread);
  } else {
    threads->add_tail(thread);
  }
  return thread;
}
