/*
   kernel/lib/list.cpp
   Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <list.h>

List::List(void *data)
{
  this->data = data;
  prev = this;
  next = this;
}

/* -данный узел удаляется
   -правятся ссылки в prev и next
   ВНИМАНИЕ! данные в data - не удаляются! */
List::~List()
{
  prev->next = next;
  next->prev = prev;
}

/* добавляет новый узел в начало списка (сразу после this) */
void List::add(void *data)
{
  List *list = new List(data);

  list->next = next;
  next->prev = list;

  list->prev = this;
  this->next = list;
}

/* добавляет новый узел в конец списка (перед this) */
void List::add_tail(void *data)
{
  List *list = new List(data);

  list->prev = prev;
  prev->next = list;

  list->next = this;
  this->prev = list;
}

/* удаляет узел из текущего списка, и добавляет его в другой список после
   элемента head (начало списка) */
void List::move(List * head)
{
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
void List::move_tail(List * head)
{
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
int List::empty()
{
  return next == this;
}

/* служит для объединения двух не перекрывающихся списков
   вставляет данный список в другой список после узла head */
void List::splice(List * head)
{
  prev->next = head->next;
  prev->next->prev = prev;

  head->next = this;
  this->prev = head;
}

/* заменяет узел this на узел head */
void List::replace(List * n)
{
  n->next = this->next;
  n->prev = this->prev;
  n->next->prev = n;
  n->prev->next = n;

  this->next = this;
  this->prev = this;
}
