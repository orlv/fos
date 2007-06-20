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
#endif
}

u32_t *TProcess::CreatePageDir()
{
  u32_t i;
  u32_t *PageDir;

  /* выделим память под каталог страниц */
  PageDir = (u32_t *) kmalloc(PAGE_SIZE);

  for (i = 0; i < USER_MEM_BASE/1024; i++) {
    PageDir[i] = hal->ProcMan->kPageDir[i];
  }

  return PageDir;
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
      mmap(object, (u32_t *) (p->p_vaddr & 0xfffff000), p->p_memsz + (p->p_vaddr % PAGE_SIZE));
    }
  }

  /* Возвращаем указатель на точку входа */
  return h->e_entry;
}

u32_t TProcess::mount_page(register u32_t phys_page, register u32_t log_page)
{
  return map_page(phys_page, log_page, PageDir, 1);
}

u32_t TProcess::umount_page(register u32_t log_page)
{
  return umap_page(log_page, PageDir);
}

void *TProcess::mem_alloc(register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;
  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = get_page();
    printk("{0x%X}", phys_pages[i]);
  }

  while(1);
  void *ptr =  this->mem_alloc(phys_pages, pages_cnt);

  if(!ptr){
    for(i = 0; i < pages_cnt; i++){
      kmfree((void *)(phys_pages[i] * PAGE_SIZE), 1);
    }
  }
  
  return ptr;
}

void *TProcess::mem_alloc_phys(register u32_t phys_address, register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;
  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = phys_address / PAGE_SIZE;
    phys_address += PAGE_SIZE;
  }

  return this->mem_alloc(phys_pages, pages_cnt);
}

/* смонтировать набор физических страниц в любое свободное место */
void *TProcess::mem_alloc(register u32_t *phys_pages, register size_t pages_cnt)
{
  task_mem_block_t *p;
  size_t size = pages_cnt * PAGE_SIZE;
  List *curr  = FreeMem;
  
  /* ищем свободный блок подходящего размера */
  while(1){
    p = (task_mem_block_t *) curr->data;
    if (p->size >= size) {
      break;
    }
    curr = curr->next;
    if(curr == FreeMem){
      printk("TaskMem: can't allocate %d bytes of memory!\n", size);
      return 0;
    }
  };

  offs_t block;
  
  if (p->size > size) {	/* используем только часть блока */
    block = p->vptr;
    p->vptr += size;		/* скорректируем указатель на начало блока */
    p->size -= size;		/* вычтем выделяемый размер */
  } else {			/* (p->size == size) */
    /* при выделении используем весь блок (запись о нём удаляется из списка свободных блоков) */
    block = p->vptr;
    delete(u32_t *) curr->data;
    delete curr;
  }

  map_pages(phys_pages, (u32_t) block / PAGE_SIZE, pages_cnt);

  task_mem_block_t *ublock = new task_mem_block_t;
  ublock->phys_pages = phys_pages;
  ublock->vptr = block;
  ublock->size = size;
    
  if (UsedMem)
    UsedMem->add_tail(ublock);
  else
    UsedMem = new List(ublock);
  
  return (void *)block;
}

/* выделить набор физических страниц и смонтировать в конкретное место */
void *TProcess::mmap(register size_t size, register void *log_address)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;

  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = get_page() / PAGE_SIZE;
  }

  void *ptr = do_mmap(phys_pages, log_address, pages_cnt);

  if(!ptr){
    for(i = 0; i < pages_cnt; i++){
      kmfree((void *)(phys_pages[i] * PAGE_SIZE), 1);
    }
  }

  return ptr;
}

/* смонтировать набор физических страниц в конкретную область памяти */
void *TProcess::mmap(register void *phys_address, register void *log_address, register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;

  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = (u32_t)phys_address / PAGE_SIZE;
    phys_address = (void *) ((u32_t)phys_address + PAGE_SIZE);
  }

  return do_mmap(phys_pages, log_address, pages_cnt);
}

/* смонтировать набор физических страниц в конкретную область памяти */
void *TProcess::do_mmap(register u32_t *phys_pages, register void *log_address, register size_t pages_cnt)
{
  task_mem_block_t *p;
  size_t size = pages_cnt * PAGE_SIZE;
  List *curr = FreeMem;

  while(1) {
    p = (task_mem_block_t *) curr->data;
    if ((p->vptr <= (u32_t) log_address) && (p->vptr + p->size >= (u32_t) log_address + size)) {
      break;
    }
    curr = curr->next;
    if ((curr == FreeMem) || (p->vptr >= (u32_t) log_address)) {
      printk("TaskMem: can't map %d bytes to 0x%X!\n", size, log_address);
      return 0;
    }
  };

  if (p->vptr == (u32_t) log_address) {
    if (p->size == size) {
      delete(u32_t *) curr->data;
      delete curr;
    } else {
      p->vptr += size;
      p->size -= size;
    }
  } else {
    size_t asize = p->size;
    p->size = (u32_t) log_address - (u32_t) p->vptr;
    if (size + p->vptr > (u32_t) log_address + size) {
      task_mem_block_t *b = new task_mem_block_t;
      b->vptr = (u32_t) log_address + size;
      b->size = asize - size - p->size;
      curr->add(b);
    }
  }

  map_pages(phys_pages, (u32_t) log_address / PAGE_SIZE, pages_cnt);
  
  task_mem_block_t *ublock = new task_mem_block_t;
  ublock->phys_pages = phys_pages;
  ublock->vptr = (u32_t) log_address;
  ublock->size = size;

  if (UsedMem)
    UsedMem->add_tail(ublock);
  else
    UsedMem = new List(ublock);

  return log_address;
}

void TProcess::map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n)
{
  u32_t i;
  
  for (i = 0; i < n; i++) {
    mount_page(phys_pages[i], log_page);
    log_page++;
  }
}

void TProcess::umap_pages(register u32_t *log_pages, register size_t n)
{
  u32_t i;

  for (i = 0; i < n ; i++) {
    umount_page(log_pages[i]);
  }
}


void TProcess::mem_free(register void *ptr)
{
  List *curr = UsedMem;
  task_mem_block_t *p;

  /* выяснить size */
  while(1) {
    p = (task_mem_block_t *) curr->data;
    if (p->vptr == (u32_t) ptr) {
      delete curr;
      break;
    }

    curr = curr->next;
    if (curr == UsedMem) {
      printk("Deleting non-allocated page!\n");
      return;
    }
  }

  umap_pages(p->phys_pages, p->size / PAGE_SIZE);
  for(u32_t i = 0; i < p->size / PAGE_SIZE; i++){
    kmfree((void *)(p->phys_pages[i] * PAGE_SIZE), 1);
  }

  curr = FreeMem;

  task_mem_block_t *c;
  task_mem_block_t *next;
  /* ищем, куда добавить блок */
  do {
    c = (task_mem_block_t *) (curr->data);

    /* слить с верхним соседом */
    if (c->vptr == p->vptr + p->size) {
      c->vptr = p->vptr;
      c->size += p->size;
      delete p;
      return;
    }

    /* слить с нижним соседом */
    if ((c->vptr + c->size == p->vptr)) {
      c->size += p->size;
      delete p;

      next = (task_mem_block_t *) (curr->next->data);
      /* и нижнего с верхним соседом */
      if (c->vptr + c->size == next->vptr) {
	c->size += next->size;
	delete next;
	delete curr->next;
      }
      return;
    }

    next = (task_mem_block_t *) (curr->next->data);
    /* разместить между нижним и верхним соседями */
    if ((c->vptr + c->size < p->vptr) && (next->vptr > p->vptr + p->size)) {
      curr->add(p);
      return;
    }

    curr = curr->next;
  } while (curr != FreeMem);

  FreeMem->add_tail(p);
}

void TProcess::mem_init(offs_t base, size_t size)
{
  task_mem_block_t *block = new task_mem_block_t;

  block->vptr = base;
  block->size = size;

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

  kmfree((void *)stack_pl0, STACK_SIZE);
  delete tss;
}

void Thread::set_tss(register off_t eip)
{
  if (!(tss = (struct TSS *)kmalloc(sizeof(struct TSS))))
    hal->panic("No memory left to create tss.");

  tss->cr3 = (u32_t)process->PageDir;
  tss->eip = eip;

  tss->eflags = 0x00000202;

  stack_pl0 = (u32_t)kmalloc(STACK_SIZE);
  tss->esp0 = stack_pl0 + STACK_SIZE - 1;

  if(!(flags & FLAG_TSK_KERN)){ /* user process */
    tss->esp = tss->ebp = (u32_t)process->mem_alloc(STACK_SIZE) + STACK_SIZE - 1;
  
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
