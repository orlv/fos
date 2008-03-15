/*
  include/list.h
  Copyright (C) 2006-2007 Oleg Fedorov

  (Sat Jan 12 20:08:59 2008) дополнен дополнительным конструктором.
*/
#ifndef _CPP_LIST_H
#define _CPP_LIST_H

#include <types.h>

/*
  Элемент кольцевого связанного списка
  Все элементы равнозначны. Подробности - см. связанные списки в Linux 2.6
*/
template <class ListItem> class List {
 public:
  List * volatile prev;
  List * volatile next;

  ListItem item;

  List() {
    this->item = 0;
    prev = this;
    next = this;
  }
  
  List(ListItem item) {
    this->item = item;
    prev = this;
    next = this;
  }

  /*
    -данный узел удаляется
    -правятся ссылки в prev и next
    ВНИМАНИЕ! данные в item - не удаляются!
  */
  ~List() {
    prev->next = next;
    next->prev = prev;
  }
  
  /* добавляет новый узел в начало списка (сразу после this) */
  List<ListItem> * add(ListItem item) {
    List<ListItem> *list = new List<ListItem>(item);
    
    list->next = next;
    next->prev = list;
    
    list->prev = this;
    this->next = list;
    return list;
  }  
  
  /* добавляет новый узел в конец списка (перед this) */
  List<ListItem> * add_tail(ListItem item) {
    List<ListItem> *list = new List<ListItem>(item);
    
    list->prev = prev;
    prev->next = list;
    
    list->next = this;
    this->prev = list;
    return list;
  }
 
  /* удаляет узел из текущего списка, и добавляет его в другой список после
     элемента head (начало списка) */
  void move(register List<ListItem> * head) {
    /* удаляемся из текущего списка */
    prev->next = this->next;
    next->prev = this->prev;

    /* добавляемся в начало другого списка */
    this->next = head->next;
    this->prev = head;
    prev->next = this;
    next->prev = this;
  }

  /* удаляет узел из текущего списка, и добавляет его в другой список перед
     элементом head (в конец списка) */
  void move_tail(register List<ListItem> * head) {
    /* удаляемся из текущего списка */
    prev->next = this->next;
    next->prev = this->prev;

    /* добавляемся в конец другого списка */
    this->next = head;
    this->prev = head->prev;
    next->prev = this;
    prev->next = this;
  }

  /* возвращает ненулевое значение, если список пуст, и нулевое значение
     в противном случае */
  int empty() {
    return next == this;
  }

  /* служит для объединения двух не перекрывающихся списков
     вставляет данный список в другой список после узла head */
  void splice(register List<ListItem> * head) {
    prev->next = head->next;
    prev->next->prev = prev;

    head->next = this;
    this->prev = head;
  }

  /* заменяет узел this на узел head */
  void replace(register List<ListItem> * head) {
    head->next = this->next;
    head->prev = this->prev;
    head->next->prev = head;
    head->prev->next = head;

    this->next = this;
    this->prev = this;
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
