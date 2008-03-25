/*
    kernel/main/memory/heap.h
    Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _MEMORY_HEAP_H
#define _MEMORY_HEAP_H

#include <types.h>
#include <fos/page.h>

#define HEAP_RESERVED_BLOCK_SIZE PAGE_SIZE

struct HeapMemBlock{
  union {
    struct{
      struct HeapMemBlock *next;
      size_t  size;
    };
    u8_t align[16];
  };
};

#endif
