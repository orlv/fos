/*
  kernel/main/procman/process.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <fos/process.h>
#include <fos/mm.h>
#include <fos/mmu.h>
#include <fos/procman.h>
#include <fos/printk.h>
#include <fos/fos.h>
#include <sys/elf32.h>
#include <string.h>

TProcess::TProcess()
{
  /* empty */
}

TProcess::~TProcess()
{
  delete memory;
  delete name;

  List<Thread *> *curr, *n;
  list_for_each_safe(curr, n, threads){
    //delete curr->item;
    delete curr;
  }
  //delete threads->item;
  delete threads;
}

u32_t TProcess::LoadELF(register void *image)
{
  u32_t *object;
  Elf32_Phdr *p;
  Elf32_Ehdr *h = (Elf32_Ehdr *) image; /* образ ELF */
  Elf32_Phdr *ph = (Elf32_Phdr *) ((u32_t) h + (u32_t) h->e_phoff);

  /*
    Если секция обозначена как загружаемая - выделим для неё память,
    скопируем из файла данные и смонтируем в адресное пространство процесса
   */
  for (p = ph; p < ph + h->e_phnum; p++){
    if (p->p_type == ELF32_TYPE_LOAD && p->p_memsz) {
      //printk("flags=%d, addr=0x%X, type=%d, fileoffs=0x%X, filesz=0x%X, memsz=0x%X \n",
      //p->p_flags, p->p_vaddr, p->p_type, p->p_offset, p->p_filesz, p->p_memsz);

      /*
	Выделим память под секцию
	Учтём, что начало секции может быть не выровнено по началу страницы

	NOTE: выделяются свободные страницы, мапятся в область ядра
	После, эти же страницы мапятся в область конкретного процесса.
	__ВАЖНО__: Затем необходимо освободить область ядра от этих страниц (kfree())
	kfree() вызовет umap_page() для каждой страницы, но umap не будет
	добавлять их в пул свободных страниц - он проверяет количество использований
	каждой страницы. Страницы освободятся только при отсоединении их от области процесса.
       */
      size_t o_size = p->p_memsz + (p->p_vaddr % PAGE_SIZE);
      object = (u32_t *) kmalloc(o_size);

      if (p->p_filesz > 0) {
	memcpy((u32_t *) ((u32_t)object + (p->p_vaddr % PAGE_SIZE)), (u32_t *) ((u32_t) image + p->p_offset), p->p_filesz);
      }

      /* Монтируем секцию в адресное пространство процесса */
      memory->mmap(p->p_vaddr & 0xfffff000, p->p_memsz + (p->p_vaddr % PAGE_SIZE), MAP_FIXED, (off_t)object, system->kmem);
      kfree(object, o_size); /* освобождаем память ядра от ненужных тут страниц */
    }
  }
  /* Возвращаем указатель на точку входа */
  return h->e_entry;
}


Thread *TProcess::thread_create(off_t eip, u16_t flags,	void * kernel_stack, void * user_stack, u16_t code_segment, u16_t data_segment)
{
  Thread *thread = new Thread(this, eip, flags, kernel_stack, user_stack , code_segment, data_segment);

  if(!threads){
    threads = new List<Thread *>(thread);
  } else {
    __mt_disable();
    threads->add_tail(thread);
    __mt_enable();
  }
  return thread;
}
