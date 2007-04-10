/*
  kernel/main/mtask/tasks.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <mm.h>
#include <paging.h>
#include <tasks.h>
#include <stdio.h>
#include <system.h>
#include <traps.h>
#include <drivers/char/timer/timer.h>
#include <io.h>
#include <dt.h>
#include <fs.h>
#include <elf32.h>
#include <string.h>

asmlinkage void scheduler_starter();
asmlinkage u32_t terminate(pid_t pid);
extern TTime *SysTimer;
extern TProcMan *ProcMan;

volatile u32_t top_pid = 0;

u32_t get_pid()
{
  u32_t res = top_pid;
  top_pid++;
  return res;
}

/* Хорошо бы этот класс в будущем преобразовать в ProcessFS */
TProcMan::TProcMan()
{
  ProcMan = this;
  extern u32_t *mpagedir;
  kPageDir = mpagedir;

  /* --------- init i386  multitasking ------------------ */
  TProcess *rootproc = new TProcess(0, 0, 0, 1);

  rootproc->tss = (struct TSS *)kmalloc(sizeof(struct TSS));
  rootproc->tss->cr3 = load_cr3();
  rootproc->tss->trace = 0;
  rootproc->tss->io_map_addr = 0;
  rootproc->tss->IOPB = 0xffffffff;
  rootproc->tss->ldtr = 0;

  rootproc->tss->cs = KERNEL_CODE;
  rootproc->tss->ds = KERNEL_DATA;
  rootproc->tss->ss = KERNEL_DATA;
  rootproc->tss->es = KERNEL_DATA;

  rootproc->flags = FLAG_TSK_READY;

  rootproc->pid = get_pid();

  proclist = new List(rootproc);

  setup_tss(&rootproc->descr, (off_t) rootproc->tss);
  DTman->load_tss(BASE_TSK_SEL_N, &rootproc->descr);

  ltr(BASE_TSK_SEL);
  lldt(0);

  /* --------------------------------------------------- */
  TProcess *sched = NewLightProc((off_t) & start_sched, 0);
  DTman->load_tss(BASE_TSK_SEL_N + 1, &sched->descr);
}

u32_t TProcMan::exec(void *image)
{
  u32_t *PageDir = CreatePageDir();
  TProcess *Process = new TProcess(image, FLAG_TSK_READY, PageDir, 0);
  add(Process);
  return 0;
}

/* Добавляет процесс в список процессов */
void TProcMan::add(TProcess * process)
{
  proclist->add_tail(process);
}

void TProcMan::del(List * proc)
{
  delete(TProcess *) proc->data;
  delete proc;
}

res_t TProcMan::stop(pid_t pid)
{
#warning TODO: FIX!!!!!!!!!!!!!!!
#if 0
  TListEntry *first = proclist->FirstEntry;
  TListEntry *iter = first;
  TProcess *proc;
  while (1) {
    proc = (TProcess *) iter->data;
    if (proc->pid == pid) {
      proc->flags = (proc->flags) | FLAG_TSK_TERM;
      break;
    }
    if (iter->next = first)
      return RES_FAULT;
    iter = iter->next;
  };
  return RES_SUCCESS;
#endif
  return RES_FAULT;
}

TProcess *TProcMan::get_process_by_pid(u32_t pid)
{
  List *current = proclist;

  do {
    if (((TProcess *) current->data)->pid == pid)
      return (TProcess *) current->data;

    current = current->next;
  } while (current != proclist);

  return 0;
}

#define HELLOW_STRING "[FOS]"

TProcess::TProcess(void *image, u16_t flags, u32_t * PageDir, u32_t crutch)
{
  pid = get_pid();
  /* ------------- */
  struct message *_msg = new(struct message);

  _msg->send_size = strlen(HELLOW_STRING);
  _msg->send_buf = new char[_msg->send_size];
  strcpy((char *)_msg->send_buf, HELLOW_STRING);

  msg = new List(_msg);
  /* ------------- */
  if (crutch)
    return;

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
  set_tss(eip, (u32_t) PageDir, 0);

  //  P->pid = sel-BASE_TSK_SEL_N;
  this->flags = flags;
}

u32_t TProcess::LoadELF(void *image)
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
	panic("666");
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

u32_t TProcess::mount_page(u32_t phys_page, u32_t log_page)
{
  return k_mount_page(phys_page, log_page, PageDir, 1);
}

u32_t TProcess::umount_page(u32_t log_page)
{
  return k_umount_page(log_page, PageDir);
}

void TProcess::set_stack_pl0()
{
  if (!(stack_pl0 = (off_t) kmalloc(STACK_SIZE)))	/* Не удалось выделить память */
    panic("No memory left.");

  stack_pl0 += STACK_SIZE - 1;	/* Пусть указатель указывает на конец стека */
}

void TProcess::set_tss(off_t eip, u32_t cr3, u32_t crunch)
{
  if (!(tss = (struct TSS *)kmalloc(sizeof(struct TSS))))
    panic("No memory left.");

  /* Заполним TSS */
  tss->cr3 = cr3;
  tss->eip = eip;
  tss->trace = 0;

  tss->eflags = 0x00000202;
  tss->eax = 0;
  tss->ecx = 0;
  tss->edx = 0;
  tss->ebx = 0;

  if (crunch) {
    tss->esp = stack_pl0;
    tss->ebp = stack_pl0;
  } else {
    u32_t stack_pl3 = (u32_t) kmalloc(PAGE_SIZE);
    mount_page(((u32_t) stack_pl3) / PAGE_SIZE, 0xE0000);
    stack_pl3 = 0xE0000000 + PAGE_SIZE - 1;
    tss->esp = stack_pl3;
    tss->ebp = stack_pl3;

    tss->esp0 = stack_pl0;
  }

  tss->esi = 0;
  tss->edi = 0;

  if (crunch) {
    tss->cs = KERNEL_CODE;
    tss->es = KERNEL_DATA;
    tss->ss = KERNEL_DATA;
    tss->ds = KERNEL_DATA;
  } else {
    tss->cs = USER_CODE;
    tss->es = USER_DATA;
    tss->ss = USER_DATA;
    tss->ds = USER_DATA;
    tss->ss0 = KERNEL_DATA;
  }

  tss->fs = 0;
  tss->gs = 0;
  tss->ldtr = 0;
  tss->trace = 0;
  tss->io_map_addr = 0;
  tss->IOPB = 0xffffffff;

  /* Установим TSS */
  setup_tss(&descr, (off_t) tss);
}

void TProcess::run()
{
  DTman->load_tss(BASE_TSK_SEL_N, &descr);
  asm("ljmp $0x38, $0");
}

void *TProcess::mem_alloc(size_t size)
{
  /* выделим страницы памяти */
  offs_t ptr = (offs_t) kmalloc(size);
  if (!ptr)
    panic("TaskMem: Can't allocate memory!");

  return this->mem_alloc(ptr, size);
}

void *TProcess::mem_alloc(offs_t ph_ptr, size_t size)
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

void *TProcess::mem_alloc(void *ptr, size_t size, void *ph_ptr)
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
      panic("TaskMem: Can't allocate memory!");
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

void TProcess::mem_free(void *ptr)
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
  Создаёт "облегчённый" процесс: выполняющийся в том же адресном пространстве
  (не имеет собственного каталога страниц)
*/
TProcess *TProcMan::NewLightProc(off_t eip, u16_t flags)
{
  TProcess *proc = new TProcess(0, 0, 0, 1);
  if (!proc)
    return 0;			/* Не удалось выделить память */

  proc->set_stack_pl0();
  proc->set_tss(eip, load_cr3(), 1);

  //  proc->pid = sel-BASE_TSK_SEL_N;
  proc->flags = flags | FLAG_TSK_LIGHT;

  /* Зарегистрируем процесс */
  proclist->add_tail(proc);
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
