/*
  kernel/main/memory/vmm.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos/vmm.h>
#include <fos/mm.h>
#include <fos/printk.h>
#include <fos/system.h>
#include <fos/pager.h>

/*
  требования к работе mmap:

  - если запрашиваемый создаваемый регион:
  а) выходит за пределы физической памяти -- возвращается ошибка
  б) пересекается с существующим регионом -- регион создается в любом свободном месте адресного пространства

  - округление желаемого адреса начала региона (align PAGE_SIZE) производится в большую сторону (т.е. 0x1001 к 0x2000)
  
 */

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

off_t VMM::alloc_free_area(register size_t &length)
{
  memblock *p;
  preempt_disable();
  List<memblock *> *curr  = FreeMem;
  /* ищем свободный блок подходящего размера */
  while(1){
    p = curr->item;
    if (p->size >= length)
      break;
    curr = curr->next;
    if(curr == FreeMem){
      preempt_enable();
      printk("VMM: can't allocate %d bytes of memory!\n", length);
      length = 0;
      return 0;
    }
  }

  off_t vptr = p->vptr;

  if (p->size > length) {  /* используем только часть блока */
    p->vptr += length;	   /* скорректируем указатель на начало блока */
    p->size -= length;
  } else {                 /* (p->size == length) */
    /* при выделении используем весь блок */
    if(curr == FreeMem)
      FreeMem = curr->next;
    delete curr->item;
    delete curr;
  }

  preempt_enable();
  return vptr;
}

/* вырезает из списка свободных блоков указанную область */
off_t VMM::cut_free_area(off_t start, size_t &length)
{
  memblock *p;
  preempt_disable();
  List<memblock *> *curr = FreeMem;
  /* поиск необходимого блока */
  while(1) {
    p = curr->item;
    //printk("vptr=0x%X, size=0x%X\n", p->vptr, p->size);
    if ((p->vptr <= start) && (p->vptr + p->size >= start + length)) {
      break;
    }
    curr = curr->next;
    if ((curr == FreeMem) || (p->vptr >= start)) {
      preempt_enable();
      printk("VMM: can't alloc %d bytes starting from 0x%X!\n", length, start);
      length = 0;
      return 0;
    }
  }

  /* проверка, выделять весь блок, или только часть */
  if (p->vptr == start) {
    if (p->size == length) {
      if(curr == FreeMem)
	FreeMem = curr->next;
      delete curr->item;
      delete curr;
    } else {
      p->vptr += length;
      p->size -= length;
    }
  } else {
    if (p->vptr + p->size > start + length) {
      memblock *b = new memblock;
      b->vptr = start + length;
      b->size = p->size - length - (start - p->vptr);
      curr->add(b);
      //printk("*bvptr=0x%X, bsize=0x%X\n", b->vptr, b->size);
    }
    p->size = start - p->vptr;
  }
  //printk("*vptr=0x%X, size=0x%X\n", p->vptr, p->size);
  preempt_enable();
  return start;
}

/*
  переменная start:
  - если не указана (и нет флага MAP_FIXED) - выделяем блок памяти в любом свободном месте
  
  переменная from_start:

  а) если vm_from=0, используется, как первая страница последовательности физ. страниц
  б) если vm_from='адресное пространство', определяем из него последовательность физ. страниц
 */
void *VMM::mmap(register off_t start, register size_t length, register int flags, off_t from_start, VMM *vm_from)
{
  //printk("start=0x%X, length=0x%X, flags=0x%X, from_start=0x%X, vm_from=0x%X\n", start, length, flags, from_start, vm_from);
  if(!length) return 0;

  length = ALIGN(length, MM_MINALLOC);

  if(start%MM_MINALLOC)
    if((flags & MAP_FIXED)) {
      printk("VMM: error allocating not-aligned fixed region 0x%X\n", start);
      return 0;
    } else
      start += MM_MINALLOC - start%MM_MINALLOC;
  
  if(!start && !(flags & MAP_FIXED)) { /* если start не указан (и нет флага MAP_FIXED) - выделяем в любом свободном месте */
    start = alloc_free_area(length);
    if(!length) return 0;
  } else {
    /* проверяем доступность блока памяти */
    if(start && ((start + length > mem_base + mem_size) || (start + length < start) || (start < mem_base))) {
      printk("VMM: overflow [start=0x%X, length=0x%X]\n", start, length);
      return 0;
    }
    size_t l = length;
    start = cut_free_area(start, l);
    if(!l)
      if((flags & MAP_FIXED))
	return 0;
      else {
	start = alloc_free_area(length);
	if(!length) return 0;
      }
  }

  /* если мы здесь - значит область для маппинга успешно найдена, можно мапить */
  if(from_start || (flags & MAP_FIXED))
    if(vm_from) { /* монтируем конкретные физ. стр. из другого адр. пр-ва */
      u32_t *pagedir = vm_from->pager->pagedir;
      for(u32_t vpage = PAGE(start), ppage = PAGE(from_start), end = vpage + PAGE(length);
	  vpage < end; vpage++, ppage++)
	pager->mount_page(PAGE(OFFSET(phys_addr_from(ppage, pagedir))), vpage);
    } else { /* монтируем последовательность физ. страниц, начиная с адреса phys_start */
      for(u32_t vpage = PAGE(start), ppage = PAGE(from_start), end = vpage + PAGE(length);
	  vpage < end; vpage++, ppage++){
	//printk("ppage=0x%X, vpage=0x%X\n", ppage, vpage);
	pager->mount_page(ppage, vpage);}
    }
  else {
    if(flags & MAP_DMA16) /* выделяем и монтируем страницы и зоны DMA16 */
      for(u32_t vpage = PAGE(start), end = vpage + PAGE(length); vpage < end; vpage++)
	pager->mount_page(get_page_DMA16(), vpage);
    else
      /* выделяем и монтируем любые свободные физические страницы */
      for(u32_t vpage = PAGE(start), end = vpage + PAGE(length); vpage < end; vpage++)
	pager->mount_page(get_page(), vpage);
  }

  return ADDRESS(start);
}

int VMM::munmap(register off_t start, register size_t length)
{
  if((start < mem_base) || (start + length > mem_base + mem_size))
    return 0;
  length = ((length + MM_MINALLOC - 1) / MM_MINALLOC) * PAGE_SIZE;
  u32_t vptr = start;

  preempt_disable();  
  /* проверим, смонтированы ли страницы */
  #warning данный код следует оптимизировать
  if(!check_pages(PAGE(vptr), pager->pagedir, PAGE(length))) {
    preempt_enable();
    printk("Trying to delete non-allocated page(s)!\n");
    return 0;
  }
   
  //printk("VMM: freeing 0x%X bytes, starting (virtual) 0x%X\n", length, start);
  pager->umap_pages(PAGE(vptr), PAGE(length));

  List<memblock *> *curr = FreeMem;
  memblock *c;
  memblock *next;

  /* ищем, куда добавить блок */
  if(FreeMem->item->vptr > vptr + length) {
    memblock *p = new memblock;
    p->vptr = vptr;
    p->size = length;
    FreeMem = FreeMem->add_tail(p);
    c = FreeMem->item;
    preempt_enable();
    return length;
  }

  do{
    c = curr->item;
    /* слить с верхним соседом */
    if (c->vptr == vptr + length) {
      c->vptr = vptr;
      c->size += length;
      preempt_enable();
      return length;
    }
    
    /* слить с нижним соседом */
    if (c->vptr + c->size == vptr) {
      c->size += length;

      next = curr->next->item;
      /* и нижнего с верхним соседом */
      if (c->vptr + c->size == next->vptr) {
	c->size += next->size;
	delete next;
	delete curr->next;
      }
      preempt_enable();
      return length;
    }

    next = curr->next->item;
    /* разместить между нижним и верхним соседями */
    if ((c->vptr + c->size < vptr) && ((curr->next == FreeMem) ||  (next->vptr > vptr + length))) {
      memblock *p = new memblock;
      p->vptr = vptr;
      p->size = length;
      curr->add(p);
      preempt_enable();
      return length;
    }
    
    curr = curr->next;
  }while (curr != FreeMem);

  memblock *p = new memblock;
  p->vptr = vptr;
  p->size = length;
  curr->add_tail(p);
  preempt_enable();
  return length;
}
