/*
   include/stack.h
   Copyright (C) 2007 Oleg Fedorov
*/

#ifndef _STACK_H
#define _STACK_H

#include <types.h>

template <class StackItem> class Stack {
 public:
  Stack *next;
  StackItem *items;
  size_t size;
  u32_t top;

  Stack(size_t size)
    {
      this->size = size;
      items = new StackItem[size];
      top = 0;
    }

  /* ВНИМАНИЕ! данные в item - не удаляются! */
  ~Stack()
    {
      delete items;
    }

  void push(StackItem item)
  {
    if(top == size){
      //while(1) asm("incb 0xb8000+158\n" "movb $0x5f,0xb8000+159");
      resize(size + 32);
    }
    
    items[top] = item;
    top++;
  }  
  
  StackItem pop()
  {
    if(top == 0)
      return 0;

    top--;
    return items[top];
  }

  void resize(size_t newsize)
  {
    items = (StackItem *)realloc(items, sizeof(StackItem)*newsize);
    size = newsize;
  }
};

#endif
