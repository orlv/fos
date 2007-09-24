/*
  kernel/main/memory/memory.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/printk.h>
#include <fos/process.h>
#include <fos/hal.h>
#include <fos/pager.h>

/*
  требования к работе mmap:

  - если запрашиваемый создаваемый регион:
  а) выходит за пределы физической памяти -- возвращается ошибка
  б) пересекается с существующим регионом -- регион создается в любом свободном месте адресного пространства

  - округление желаемого адреса начала региона (align PAGE_SIZE) производится в большую сторону (т.е. 0x1001 к 0x2000)
  
 */

void mm_srv()
{
  Thread *thread;
  struct message *msg = new message;
  msg->tid = 0;
  while (1) {
    msg->recv_size = 0;
    msg->tid = _MSG_SENDER_ANY;

    receive(msg);

    switch(msg->a0){
    case MM_CMD_MMAP:
      //printk("mm: mapping 0x%X bytes of memory to 0x%X\n", msg->a2, msg->a1);
      thread = hal->procman->get_thread_by_tid(msg->tid);
      msg->a0 = (u32_t) thread->process->memory->mmap(msg->a1 & ~0xfff, msg->a2, msg->a1 & 0xfff, msg->a3, 0);
      msg->send_size = 0;
      reply(msg);
      break;

    case MM_CMD_MUNMAP:
      //printk("mm: unmapping 0x%X bytes from 0x%X\n", msg-a2, msg->a1);
      thread = hal->procman->get_thread_by_tid(msg->tid);
      if(msg->a1 > USER_MEM_BASE){
	msg->a0 = 1;
	thread->process->memory->munmap(msg->a1, msg->a2); //mem_free((void *)msg->a1, msg->a2);
      } else
	msg->a0 = -1;
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

VMM::VMM(offs_t base, size_t size)
{
  memblock *block = new memblock;

  block->vptr = base;
  block->size = size;

  FreeMem = new List<memblock *>(block);

  mem_base = base;
  mem_size = size;
}

VMM::~VMM()
{
  List<memblock *> *curr, *n;

  /* удалим список свободной памяти */
  list_for_each_safe (curr, n, FreeMem) {
    delete curr->item;
    delete curr;
  }

  delete FreeMem->item;
  delete FreeMem;
  delete pager;
}

off_t VMM::alloc_free_area(register size_t &lenght)
{
  memblock *p;
  __mt_disable();
  List<memblock *> *curr  = FreeMem;
  /* ищем свободный блок подходящего размера */
  while(1){
    p = curr->item;
    if (p->size >= lenght)
      break;
    curr = curr->next;
    if(curr == FreeMem){
      __mt_enable();
      printk("VMM: can't allocate %d bytes of memory!\n", lenght);
      lenght = 0;
      return 0;
    }
  }

  off_t vptr = p->vptr;

  if (p->size > lenght) {  /* используем только часть блока */
    p->vptr += lenght;	   /* скорректируем указатель на начало блока */
    p->size -= lenght;
  } else {                 /* (p->size == lenght) */
    /* при выделении используем весь блок */
    if(curr == FreeMem)
      FreeMem = curr->next;
    delete curr->item;
    delete curr;
  }

  __mt_enable();
  return vptr;
}

/* вырезает из списка свободных блоков указанную область */
off_t VMM::cut_free_area(off_t start, size_t &lenght)
{
  memblock *p;
  __mt_disable();
  List<memblock *> *curr = FreeMem;
  /* поиск необходимого блока */
  while(1) {
    p = curr->item;
    //printk("vptr=0x%X, size=0x%X\n", p->vptr, p->size);
    if ((p->vptr <= start) && (p->vptr + p->size >= start + lenght)) {
      break;
    }
    curr = curr->next;
    if ((curr == FreeMem) || (p->vptr >= start)) {
      __mt_enable();
      printk("VMM: can't alloc %d bytes starting from 0x%X!\n", lenght, start);
      lenght = 0;
      return 0;
    }
  }

  /* проверка, выделять весь блок, или только часть */
  if (p->vptr == start) {
    if (p->size == lenght) {
      if(curr == FreeMem)
	FreeMem = curr->next;
      delete curr->item;
      delete curr;
    } else {
      p->vptr += lenght;
      p->size -= lenght;
    }
  } else {
    if (p->vptr + p->size > start + lenght) {
      memblock *b = new memblock;
      b->vptr = start + lenght;
      b->size = p->size - lenght - (start - p->vptr);
      curr->add(b);
      //printk("*bvptr=0x%X, bsize=0x%X\n", b->vptr, b->size);
    }
    p->size = start - p->vptr;
  }
  //printk("*vptr=0x%X, size=0x%X\n", p->vptr, p->size);
  __mt_enable();
  return start;
}

/*
  from_start:

  а) если vm_from=0, используется, как первая страница последовательности физ. страниц
  б) если vm_from='адресное пространство', определяем из него последовательность физ. страниц
 */
void *VMM::mmap(register off_t start, register size_t lenght, register int flags, off_t from_start, VMM *vm_from)
{
  //printk("start=0x%X, lenght=0x%X, flags=0x%X, from_start=0x%X, vm_from=0x%X\n", start, lenght, flags, from_start, vm_from);
  if(!lenght) return 0;
  /* проверки выравнивания */
  if(lenght%MM_MINALLOC)
    lenght += MM_MINALLOC - lenght%MM_MINALLOC;

  if(start%MM_MINALLOC)
    if((flags & MAP_FIXED)) {
      printk("VMM: error allocating not-aligned fixed region 0x%X\n", start);
      return 0;
    } else
      start += MM_MINALLOC - start%MM_MINALLOC;
  
  if(!start && !(flags & MAP_FIXED)) { /* если start не указан (и нет флага MAP_FIXED) - выделяем в любом свободном месте */
    start = alloc_free_area(lenght);
    if(!lenght) return 0;
  } else {
    /* проверяем доступность блока памяти */
    if(start && ((start + lenght > mem_base + mem_size) || (start + lenght < start) || (start < mem_base))) {
      printk("VMM: overflow [start=0x%X, lenght=0x%X]\n", start, lenght);
      return 0;
    }
    size_t l = lenght;
    start = cut_free_area(start, l);
    if(!l)
      if((flags & MAP_FIXED))
	return 0;
      else {
	start = alloc_free_area(lenght);
	if(!lenght) return 0;
      }
  }

  /* если мы здесь - значит область для маппинга успешно найдена, можно мапить */
  if(from_start || (flags & MAP_FIXED))
    if(vm_from) { /* монтируем конкретные физ. стр. из другого адр. пр-ва */
      u32_t *pagedir = vm_from->pager->pagedir;
      for(u32_t vpage = PAGE(start), ppage = PAGE(from_start), end = vpage + PAGE(lenght);
	  vpage < end; vpage++, ppage++)
	pager->mount_page(PAGE(OFFSET(phys_addr_from(ppage, pagedir))), vpage);
    } else { /* монтируем последовательность физ. страниц, начиная с адреса phys_start */
      for(u32_t vpage = PAGE(start), ppage = PAGE(from_start), end = vpage + PAGE(lenght);
	  vpage < end; vpage++, ppage++){
	//printk("ppage=0x%X, vpage=0x%X\n", ppage, vpage);
	pager->mount_page(ppage, vpage);}
    }
  else {
    if(flags & MAP_DMA16) /* выделяем и монтируем страницы и зоны DMA16 */
      for(u32_t vpage = PAGE(start), end = vpage + PAGE(lenght); vpage < end; vpage++)
	pager->mount_page(get_page_DMA16(), vpage);
    else
      /* выделяем и монтируем любые свободные физические страницы */
      for(u32_t vpage = PAGE(start), end = vpage + PAGE(lenght); vpage < end; vpage++)
	pager->mount_page(get_page(), vpage);
  }

  return ADDRESS(start);
}

int VMM::munmap(register off_t start, register size_t lenght)
{
  if((start < mem_base) || (start + lenght > mem_base + mem_size))
    return 0;
  lenght = ((lenght + MM_MINALLOC - 1) / MM_MINALLOC) * PAGE_SIZE;
  u32_t vptr = start;

  __mt_disable();  
  /* проверим, смонтированы ли страницы */
  #warning данный код следует оптимизировать
  if(!check_pages(PAGE(vptr), pager->pagedir, PAGE(lenght))) {
    __mt_enable();
    printk("Trying to delete non-allocated page(s)!\n");
    return 0;
  }
   
  //printk("VMM: freeing 0x%X bytes, starting (virtual) 0x%X\n", lenght, start);
  pager->umap_pages(PAGE(vptr), PAGE(lenght));

  List<memblock *> *curr = FreeMem;
  memblock *c;
  memblock *next;

  /* ищем, куда добавить блок */
  if(FreeMem->item->vptr > vptr + lenght) {
    memblock *p = new memblock;
    p->vptr = vptr;
    p->size = lenght;
    FreeMem = FreeMem->add_tail(p);
    c = FreeMem->item;
    __mt_enable();
    return lenght;
  }

  do{
    c = curr->item;
    /* слить с верхним соседом */
    if (c->vptr == vptr + lenght) {
      c->vptr = vptr;
      c->size += lenght;
      __mt_enable();
      return lenght;
    }
    
    /* слить с нижним соседом */
    if (c->vptr + c->size == vptr) {
      c->size += lenght;

      next = curr->next->item;
      /* и нижнего с верхним соседом */
      if (c->vptr + c->size == next->vptr) {
	c->size += next->size;
	delete next;
	delete curr->next;
      }
      __mt_enable();
      return lenght;
    }while (curr != FreeMem);

    next = curr->next->item;
    /* разместить между нижним и верхним соседями */
    if ((c->vptr + c->size < vptr) && ((curr->next == FreeMem) ||  (next->vptr > vptr + lenght))) {
      memblock *p = new memblock;
      p->vptr = vptr;
      p->size = lenght;
      curr->add(p);
      __mt_enable();
      return lenght;
    }
    
    curr = curr->next;
  }while (curr != FreeMem);

  memblock *p = new memblock;
  p->vptr = vptr;
  p->size = lenght;
  curr->add_tail(p);
  __mt_enable();
  return lenght;
}
