/*
   include/list.h
   Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _LIST_H
#define _LIST_H

#include <types.h>

/*
  Элемент кольцевого связанного списка
  Все элементы равнозначны. Подробности - см. связанные списки в Linux 2.6
*/
class List {
public:
  List * prev;
  List *next;

  void *data;

   List(void *data);

  /* -данный узел удаляется
     -правятся ссылки в prev и next
     ВНИМАНИЕ! данные в data - не удаляются! */
  ~List();

  /* добавляет новый узел в начало списка (сразу после this) */
  void add(void *data);

  /* добавляет новый узел в конец списка (перед this) */
  void add_tail(void *data);

  /* удаляет узел из текущего списка, и добавляет его в другой список после
     элемента head (начало списка) */
  void move(List * head);

  /* удаляет узел из текущего списка, и добавляет его в другой список перед
     элементом head (в конец списка) */
  void move_tail(List * head);

  /* возвращает ненулевое значение, если список пуст, и нулевое значение
     в противном случае */
  int empty();

  /* служит для объединения двух не перекрывающихся списков
     вставляет данный список в другой список после узла head */
  void splice(List * head);

  /* заменяет узел this на узел head */
  void replace(List * head);
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
