/*
  fos/vmm.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _FOS_VMM_H
#define _FOS_VMM_H

#include <types.h>
#include <c++/list.h>

struct memblock {
  offs_t vptr; /* на какой адрес в памяти процесса смонтировано */
  size_t size; /* размер блока */
};

class VMM {
 private:
  List<memblock *> * volatile FreeMem;
  off_t alloc_free_area(register size_t &lenght);
  off_t cut_free_area(register off_t start, register size_t &lenght);

 public:
  VMM(offs_t base, size_t size);
  ~VMM();

  off_t mem_base;
  size_t mem_size;
  class Pager *pager;

  void *find_free_area(register size_t lenght);
  /* если start=0 -- ищем с find_free_area() память и мапим */
  void *mmap(register off_t start, register size_t lenght, register int flags, off_t from_start, VMM *vm_from);

  /* отсоединить страницы от данного адресного пространства
     (если станицы смонтированы также в другом месте - они там останутся) */
  int munmap(register off_t start, register size_t lenght);
};

#endif
