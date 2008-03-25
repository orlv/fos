/*
    fos/mm.h
    Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _FOS_HEAP_H
#define _FOS_HEAP_H

#include <types.h>

#define HEAP_RESERVED_BLOCK_SIZE 0x1000

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
