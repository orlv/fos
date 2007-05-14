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

extern volatile u32_t top_pid;

#define HELLOW_STRING "[FOS]"

u32_t create_pagedir();

TProcess::TProcess(register void *image, register u16_t flags, register u32_t * PageDir)
{
  pid = hal->ProcMan->get_pid();
  /* ------------- */
  struct message *_msg = new(struct message);

  _msg->send_size = strlen(HELLOW_STRING);
  _msg->send_buf = new char[_msg->send_size];
  strcpy((char *)_msg->send_buf, HELLOW_STRING);

  msg = new List(_msg);
  /* ------------- */
  this->PageDir = PageDir;
  this->mem_init();
  /* ------------- */

  off_t eip = LoadELF(image);

  mem_alloc((u32_t *) 0xf0000000, 640 * 480 * 2 / 4096, (u32_t *) 0xf0000000);
  //u32_t i;
  //for(i=0; i<(640*480*2/4096); i++)
  //mount_page(0xf0000+i, 0xf0000+i);

  set_stack_pl0();
  set_tss(eip, (u32_t) PageDir);

  //  P->pid = sel-BASE_TSK_SEL_N;
  this->flags = flags;
}

TProcess::TProcess(register u16_t flags, register u32_t * PageDir)
{
  pid = hal->ProcMan->get_pid();

  struct message *_msg = new(struct message);

  _msg->send_size = strlen(HELLOW_STRING);
  _msg->send_buf = new char[_msg->send_size];
  strcpy((char *)_msg->send_buf, HELLOW_STRING);

  msg = new List(_msg);

  this->PageDir = PageDir;
  this->flags = flags;
}


u32_t TProcess::LoadELF(register void *image)
{
  u32_t i;
  u32_t *object, *filedata;

  Elf32_Phdr *p, *ph;
  Elf32_Ehdr *h;

  h = (Elf32_Ehdr *) image;
  ph = (Elf32_Phdr *) ((u32_t) h + (u32_t) h->e_phoff);

  for (p = ph; p < ph + h->e_phnum; p++) {
    /* Если секция загружаемая */
    if (p->p_type == ELF32_TYPE_LOAD && p->p_memsz) {
      //      printk("flags=%d, addr=0x%X, type=%d, fileoffs=0x%X, filesz=0x%X, memsz=0x%X \n", p->p_flags, p->p_vaddr, p->p_type, p->p_offset, p->p_filesz, p->p_memsz);

      if (p->p_filesz > p->p_memsz) {
	printk("Invalid section!");
	hal->panic("666");
      }

      object = (u32_t *) kmalloc(p->p_memsz);
      printk("[0x%X]", object);
      /* Копируем данные */
      filedata = (u32_t *) ((u32_t) image + p->p_offset);
      //printk("sz=0x%X \n",p->p_filesz/sizeof(u32_t));

      if (p->p_filesz > 0) {
	for (i = 0; i < p->p_filesz / sizeof(u32_t) + 1; i++) {
	  object[i] = filedata[i];
	}
      }

      mem_alloc((u32_t *) (p->p_vaddr & 0xfffff000), p->p_memsz, object);

      /*
         for(i=0; i<p->p_memsz; i+=0x1000)
         {
         mount_page(((u32_t)&object[i])/PAGE_SIZE , ((u32_t)&((u32_t *)p->p_vaddr)[i])/PAGE_SIZE);
         }
       */
    }
  }

  delete(u32_t *) image;
  return h->e_entry;
}

u32_t TProcess::mount_page(register u32_t phys_page, register u32_t log_page)
{
  return k_mount_page(phys_page, log_page, PageDir, 1);
}

u32_t TProcess::umount_page(register u32_t log_page)
{
  return k_umount_page(log_page, PageDir);
}

void TProcess::set_stack_pl0()
{
  if (!(stack_pl0 = (off_t) kmalloc(STACK_SIZE)))	/* Не удалось выделить память */
    hal->panic("No memory left.");

  stack_pl0 += STACK_SIZE - 1;	/* Пусть указатель указывает на конец стека */
}

void TProcess::set_tss(register off_t eip, register u32_t cr3)
{
  if (!(tss = (struct TSS *)kmalloc(sizeof(struct TSS))))
    hal->panic("No memory left.");

  /* Заполним TSS */
  tss->cr3 = cr3;
  tss->eip = eip;

  tss->eflags = 0x00000202;

  u32_t stack_pl3 = (u32_t) kmalloc(PAGE_SIZE);
  mount_page(((u32_t) stack_pl3) / PAGE_SIZE, 0xE0000);
  stack_pl3 = 0xE0000000 + PAGE_SIZE - 1;
  tss->esp = stack_pl3;
  tss->ebp = stack_pl3;

  tss->esp0 = stack_pl0;

  tss->cs = USER_CODE;
  tss->es = USER_DATA;
  tss->ss = USER_DATA;
  tss->ds = USER_DATA;
  tss->ss0 = KERNEL_DATA;
   
  tss->IOPB = 0xffffffff;

  /* Установим TSS */
  hal->gdt->set_tss_descriptor((off_t) tss, &descr);
}

void TProcess::kprocess_set_tss(register off_t eip, register u32_t cr3)
{
  if (!(tss = (struct TSS *)kmalloc(sizeof(struct TSS))))
    hal->panic("No memory left to create tss for kernel-mode process.");

  /* Заполним TSS */
  tss->cr3 = cr3;
  tss->eip = eip;

  tss->eflags = 0x00000202;

  tss->esp = stack_pl0;
  tss->ebp = stack_pl0;

  tss->cs = KERNEL_CODE;
  tss->es = KERNEL_DATA;
  tss->ss = KERNEL_DATA;
  tss->ds = KERNEL_DATA;

  tss->IOPB = 0xffffffff;

  /* Установим TSS */
  hal->gdt->set_tss_descriptor((off_t) tss, &descr);
}

void TProcess::run()
{
  hal->gdt->load_tss(BASE_TSK_SEL_N, &descr);
  asm("ljmp $0x38, $0");
}

void *TProcess::mem_alloc(register size_t size)
{
  /* выделим страницы памяти */
  offs_t ptr = (offs_t) kmalloc(size);
  if (!ptr)
    hal->panic("TaskMem: Can't allocate memory!");

  return this->mem_alloc(ptr, size);
}

void *TProcess::mem_alloc(register offs_t ph_ptr, register size_t size)
{
  /*
     Определимся с местом под выделяемый блок в списке свободной памяти процесса
   */

  if (!size)
    return 0;
  task_mem_block_t *p = 0, *prevp;
  offs_t block;
  size_t asize;

  /* округлим объём */
  asize = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    asize += MM_MINALLOC;

  prevp = 0;
  List *curr;

  /* ищем блок подходящего размера */
  curr = FreeMem;
  do {
    if (((task_mem_block_t *) (curr->data))->size >= asize) {
      p = (task_mem_block_t *) curr->data;
      break;
    }
    curr = curr->next;
  } while (curr != FreeMem);

  if (!p) {
    printk("TaskMem: can't allocate %d bytes of memory!\n", asize);
    return NULL;
  }
  //printk("p=0x%X, p->size=0x%X, p->next=0x%X }\n", p, p->size, p->next);

  if (p->size > asize) {	/* используем только часть блока */
    block = p->ptr;
    p->ptr += asize;		/* скорректируем указатель на начало блока */
    p->size -= asize;		/* вычтем выделяемый размер */
  } else {			/* (p->size == asize) */
    /* при выделении используем весь блок */
    block = p->ptr;
    delete(u32_t *) curr->data;
    delete curr;
  }

  /* ------------------------------------------------------- */
  /* смонтируем страницы */
  u32_t i;
  u32_t l = (u32_t) block / PAGE_SIZE;
  u32_t page = ph_ptr / PAGE_SIZE;
  printk("{ph_ptr=0x%X, asize=0x%X, l=0x%X}", ph_ptr, asize, l);

  for (i = 0; i < asize / PAGE_SIZE; i++) {
    mount_page(page, l);
    page++;
    l++;
  }

  task_mem_block_t *ublock = new task_mem_block_t;
  ublock->ptr = block;
  ublock->size = asize;

  if (UsedMem)
    UsedMem->add_tail(ublock);
  else
    UsedMem = new List(ublock);

  return (void *)block;
}

void *TProcess::mem_alloc(register void *ptr, register size_t size, register void *ph_ptr)
{
  //  printk("\nptr=0x%X, size=0x%X, ph_ptr=0x%X \n", ptr, size, ph_ptr);

  if (!size)
    return 0;
  task_mem_block_t *p, *block;
  size_t asize;

  /* округлим объём */
  asize = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    asize += MM_MINALLOC;

  // printk("asize=0x%X \n", asize);

  List *curr = FreeMem;

  while (1) {
    if (!curr || !curr->data) {
      printk("TaskMem: #1 can't allocate %d bytes of memory!\n", asize);
      return 0;
    }

    p = (task_mem_block_t *) (curr->data);
    //printk("p->ptr=0x%X p->size=0x%X\n", p->ptr, p->size);
    if ((p->ptr <= (u32_t) ptr) && (p->ptr + p->size >= (u32_t) ptr + asize)) {
      break;
    }
    if (p->ptr > (u32_t) ptr) {
      printk("TaskMem: #2 can't allocate %d bytes of memory!\n", asize);
      return 0;
    }

    curr = curr->next;
  }

  if (p->ptr == (u32_t) ptr) {
    if (p->size == asize) {
      delete(u32_t *) curr->data;
      delete curr;
    } else {
      p->ptr += asize;
      p->size -= asize;
    }
  } else {
    size = p->size;
    p->size = (u32_t) ptr - (u32_t) p->ptr;
    if (size + p->ptr > (u32_t) ptr + asize) {
      task_mem_block_t *b = new task_mem_block_t;
      b->ptr = (u32_t) ptr + asize;
      b->size = size - asize - p->size;
      curr->add(b);
    }
  }

  block = (task_mem_block_t *) ptr;

  /* ------------------------------------------------------- */
  if (!ph_ptr) {
    /* выделим страницы памяти */
    void *ptr = kmalloc(asize);
    if (!ptr)
      hal->panic("TaskMem: Can't allocate memory!");
  } else {
    ptr = ph_ptr;
  }

  /* смонтируем страницы */
  u32_t i;
  u32_t l = (u32_t) block / PAGE_SIZE;
  u32_t page = (u32_t) ptr / PAGE_SIZE;
  for (i = 0; i < asize / PAGE_SIZE; i++) {
    mount_page(page, l);
    page++;
    l++;
  }

  task_mem_block_t *ublock = new task_mem_block_t;
  ublock->ptr = (u32_t) block;
  ublock->size = asize;

  if (UsedMem)
    UsedMem->add_tail(ublock);
  else
    UsedMem = new List(ublock);

  return block;
}

void TProcess::mem_free(register void *ptr)
{
  /*
     выяснить size
   */
  List *curr = UsedMem;
  task_mem_block_t *c, *n;
  offs_t size = 0;

  do {
    c = (task_mem_block_t *) curr->data;
    if (((task_mem_block_t *) curr->data)->ptr == (u32_t) ptr) {
      size = ((task_mem_block_t *) curr->data)->size;
      delete(task_mem_block_t *) curr->data;
      delete curr;
      break;
    }

    curr = curr->next;
  } while (curr != UsedMem);

  if (!size) {
    printk("Kernel: Attempting to delete non-allocated page!\n");
    return;
  }

  /* отмонтируем страницы */
  u32_t l;
  for (l = (u32_t) ptr / PAGE_SIZE; l < size / PAGE_SIZE; l++)
    umount_page(l);

  kmfree(ptr, size);

  task_mem_block_t *block = new task_mem_block_t;

  block->ptr = (u32_t) ptr;
  block->size = size;

  curr = FreeMem;

  /* ищем, куда добавить блок */
  do {
    c = (task_mem_block_t *) (curr->data);

    /* слить с верхним соседом */
    if (c->ptr == block->ptr + block->size) {
      c->ptr = block->ptr;
      c->size += block->size;
      delete block;
      return;
    }

    /* слить с нижним соседом */
    if ((c->ptr + c->size == block->ptr)) {
      c->size += block->size;
      delete block;

      n = (task_mem_block_t *) (curr->next->data);
      /* и нижнего с верхним соседом */
      if (c->ptr + c->size == n->ptr) {
	c->size += n->size;
	delete n;
	delete curr->next;
      }
      return;
    }

    n = (task_mem_block_t *) (curr->next->data);
    /* резместить между нижним и верхним соседями */
    if ((c->ptr + c->size < block->ptr) && (n->ptr > block->ptr + block->size)) {
      curr->add(block);
      return;
    }

    curr = curr->next;
  } while (curr != FreeMem);

  if (!curr) {
    FreeMem->add_tail(block);
  }
}

void TProcess::mem_init()
{
  task_mem_block_t *block = new task_mem_block_t;

  block->ptr = PROCESS_MEM_BASE;
  block->size = PROCESS_MEM_LIMIT;
  FreeMem = new List(block);

  //  UsedMem = new List(0);
}

/*
  Создаёт процесс в адресном пространстве ядра.
  Полезен для создания серверов, удобно разделяющих структуры данных с другими процессами режима ядра
*/
TProcess *TProcMan::kprocess(register off_t eip, register u16_t flags)
{
  TProcess *proc = new TProcess(0, kPageDir);

  proc->set_stack_pl0();
  proc->kprocess_set_tss(eip, load_cr3());

  proc->pid = get_pid();
  proc->flags = flags | FLAG_TSK_LIGHT;

  /* Зарегистрируем процесс */
  if(proclist)
    proclist->add_tail(proc);
  else
    proclist = new List(proc);

  return proc;
}

u32_t *TProcMan::CreatePageDir()
{
  u32_t i;
  u32_t *PageDir;

  /* выделим память под каталог страниц */
  PageDir = (u32_t *) kmalloc(PAGE_SIZE);

  for (i = 0; i < 0x3; i++) {
    PageDir[i] = kPageDir[i];
  }

  return PageDir;
};
