/*
   include/stack.h
   Copyright (C) 2007 Oleg Fedorov
*/

#ifndef _STACK_H
#define _STACK_H

#include <types.h>
#include <mm.h>

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

/**
 * list_for_each      - iterate over a list
 * @pos:        the &List to use as a loop counter.
 * @head:       the head for your list.
 */
#define list_for_each(pos, head) \
        for(pos = head->next; pos != head; pos = pos->next)

/**
 * list_for_each_prev - iterate over a list backwards
 * @pos:        the &List to use as a loop counter.
 * @head:       the head for your list.
 */
#define list_for_each_prev(pos, head) \
        for (pos = head->prev; pos != head; pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:        the &List to use as a loop counter.
 * @n:          another &List to use as temporary storage
 * @head:       the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
        for (pos = head->next, n = pos->next; pos != head; \
                pos = n, n = pos->next)

#endif
