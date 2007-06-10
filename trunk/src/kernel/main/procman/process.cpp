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

TProcess::TProcess(register u16_t flags, register void *image, register u32_t *PageDir)
{
  u32_t eip;
  if(flags & FLAG_TSK_KERN){ /* kernel process */
    this->PageDir = PageDir;
    eip = (off_t) image;
  } else {                   /* user process   */
    this->PageDir = CreatePageDir();
    this->mem_init();
    eip = LoadELF(image);
  }

  thread_create(eip, flags);
}

TProcess::~TProcess()
{
  List *curr, *n;

  /* освободим выделенную память */
  task_mem_block_t *c;
  list_for_each_safe(curr, n, UsedMem){
    c = (task_mem_block_t *) curr->data;
    //    printk("0x%X 0x%X 0x%X \n", c->vptr, c->pptr, c->size);
    kmfree((void *)c->pptr, c->size);
    delete(task_mem_block_t *) curr->data;
    delete curr;
  }

  c = (task_mem_block_t *) UsedMem->data;
  //printk("0x%X 0x%X 0x%X \n", c->vptr, c->pptr, c->size);
  kmfree((void *)c->pptr, c->size);
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
      kmfree((void *)(((u32_t)PageDir[i]) & 0xfffff000), PAGE_SIZE);
  }
  kmfree((void *)(((u32_t)PageDir) & 0xfffff000), PAGE_SIZE);
}

u32_t *TProcess::CreatePageDir()
{
  u32_t i;
  u32_t *PageDir;

  /* выделим память под каталог страниц */
  PageDir = (u32_t *) kmalloc(PAGE_SIZE);

  for (i = 0; i < USER_MEM_START/1024; i++) {
    PageDir[i] = hal->ProcMan->kPageDir[i];
  }

  return PageDir;
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
      //      printk("[0x%X]", object);
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

  //  delete(u32_t *) image;
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
    kmfree((void *)ph_ptr, size);
    return NULL;
  }
  //printk("p=0x%X, p->size=0x%X, p->next=0x%X }\n", p, p->size, p->next);

  if (p->size > asize) {	/* используем только часть блока */
    block = p->vptr;
    p->vptr += asize;		/* скорректируем указатель на начало блока */
    p->size -= asize;		/* вычтем выделяемый размер */
  } else {			/* (p->size == asize) */
    /* при выделении используем весь блок */
    block = p->vptr;
    delete(u32_t *) curr->data;
    delete curr;
  }

  /* ------------------------------------------------------- */
  /* смонтируем страницы */
  u32_t i;
  u32_t l = (u32_t) block / PAGE_SIZE;
  u32_t page = ph_ptr / PAGE_SIZE;
  // printk("{ph_ptr=0x%X, asize=0x%X, l=0x%X}", ph_ptr, asize, l);

  for (i = 0; i < asize / PAGE_SIZE; i++) {
    mount_page(page, l);
    page++;
    l++;
  }

  task_mem_block_t *ublock = new task_mem_block_t;
  ublock->pptr = ph_ptr;
  ublock->vptr = block;
  ublock->size = asize;

  if (UsedMem)
    UsedMem->add_tail(ublock);
  else
    UsedMem = new List(ublock);

  return (void *)block;
}

void *TProcess::mem_alloc(register void *ptr, register size_t size, register void *ph_ptr)
{
  //printk("\nptr=0x%X, size=0x%X, ph_ptr=0x%X \n", ptr, size, ph_ptr);

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
    if ((p->vptr <= (u32_t) ptr) && (p->vptr + p->size >= (u32_t) ptr + asize)) {
      break;
    }
    if (p->vptr > (u32_t) ptr) {
      printk("TaskMem: #2 can't allocate %d bytes of memory!\n", asize);
      return 0;
    }

    curr = curr->next;
  }

  if (p->vptr == (u32_t) ptr) {
    if (p->size == asize) {
      delete(u32_t *) curr->data;
      delete curr;
    } else {
      p->vptr += asize;
      p->size -= asize;
    }
  } else {
    size = p->size;
    p->size = (u32_t) ptr - (u32_t) p->vptr;
    if (size + p->vptr > (u32_t) ptr + asize) {
      task_mem_block_t *b = new task_mem_block_t;
      b->vptr = (u32_t) ptr + asize;
      b->size = size - asize - p->size;
      curr->add(b);
    }
  }

  block = (task_mem_block_t *) ptr;

  /* ------------------------------------------------------- */
  if (!ph_ptr) {
    /* выделим страницы памяти */
    ptr = kmalloc(asize);
    if (!ptr)
      hal->panic("TaskMem: Can't allocate memory!");
  } else {
    ptr = ph_ptr;
  }

  task_mem_block_t *ublock = new task_mem_block_t;
  ublock->pptr =(offs_t) ptr;
  ublock->vptr = (u32_t) block;
  ublock->size = asize;

  /* смонтируем страницы */
  u32_t i;
  u32_t l = (u32_t) block / PAGE_SIZE;
  u32_t page = (u32_t) ptr / PAGE_SIZE;
  for (i = 0; i < asize / PAGE_SIZE; i++) {
    mount_page(page, l);
    page++;
    l++;
  }

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
  offs_t pptr = 0;

  do {
    c = (task_mem_block_t *) curr->data;
    if (((task_mem_block_t *) curr->data)->vptr == (u32_t) ptr) {
      size = ((task_mem_block_t *) curr->data)->size;
      pptr = ((task_mem_block_t *) curr->data)->pptr;
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

  kmfree((void *)pptr, size);

  task_mem_block_t *block = new task_mem_block_t;

  block->vptr = (u32_t) ptr;
  block->size = size;

  curr = FreeMem;

  /* ищем, куда добавить блок */
  do {
    c = (task_mem_block_t *) (curr->data);

    /* слить с верхним соседом */
    if (c->vptr == block->vptr + block->size) {
      c->vptr = block->vptr;
      c->size += block->size;
      delete block;
      return;
    }

    /* слить с нижним соседом */
    if ((c->vptr + c->size == block->vptr)) {
      c->size += block->size;
      delete block;

      n = (task_mem_block_t *) (curr->next->data);
      /* и нижнего с верхним соседом */
      if (c->vptr + c->size == n->vptr) {
	c->size += n->size;
	delete n;
	delete curr->next;
      }
      return;
    }

    n = (task_mem_block_t *) (curr->next->data);
    /* резместить между нижним и верхним соседями */
    if ((c->vptr + c->size < block->vptr) && (n->vptr > block->vptr + block->size)) {
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

  block->vptr = PROCESS_MEM_BASE;
  block->size = PROCESS_MEM_LIMIT;
  FreeMem = new List(block);
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
  
  if(flags & FLAG_TSK_KERN){ /* kernel process */
    kprocess_set_tss(eip);
  } else {                            /* user process */
    set_tss(eip);
  }
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

  kmfree((void *)stack_pl0, STACK_SIZE);
  delete tss;
}

void Thread::set_tss(register off_t eip)
{
  if (!(tss = (struct TSS *)kmalloc(sizeof(struct TSS))))
    hal->panic("No memory left.");

  tss->cr3 = (u32_t)process->PageDir;
  tss->eip = eip;

  tss->eflags = 0x00000202;

  tss->esp = tss->ebp = (u32_t)process->mem_alloc(STACK_SIZE) + STACK_SIZE - 1;
  stack_pl0 = (u32_t)kmalloc(STACK_SIZE);
  tss->esp0 = stack_pl0 + STACK_SIZE - 1;

  tss->cs = USER_CODE;
  tss->es = USER_DATA;
  tss->ss = USER_DATA;
  tss->ds = USER_DATA;
  tss->ss0 = KERNEL_DATA;
   
  tss->IOPB = 0xffffffff;

  /* Установим TSS */
  hal->gdt->set_tss_descriptor((off_t) tss, &descr);
}

void Thread::kprocess_set_tss(register off_t eip)
{
  if (!(tss = (struct TSS *)kmalloc(sizeof(struct TSS))))
    hal->panic("No memory left to create tss for kernel-mode process.");

  /* Заполним TSS */
  tss->cr3 = (u32_t)process->PageDir;
  tss->eip = eip;

  tss->eflags = 0x00000202;

  stack_pl0 = (u32_t)kmalloc(STACK_SIZE);
  tss->ebp = tss->esp = stack_pl0 + STACK_SIZE - 1;

  tss->cs = KERNEL_CODE;
  tss->es = KERNEL_DATA;
  tss->ss = KERNEL_DATA;
  tss->ds = KERNEL_DATA;

  tss->IOPB = 0xffffffff;

  /* Установим TSS */
  hal->gdt->set_tss_descriptor((off_t) tss, &descr);
}

void Thread::run()
{
  hal->gdt->load_tss(BASE_TSK_SEL_N, &descr);
  asm("ljmp $0x38, $0");
}
