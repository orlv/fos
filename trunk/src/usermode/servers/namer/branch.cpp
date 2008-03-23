/*
  Copyright (C) 2006-2008 Oleg Fedorov
*/

#include <string.h>
#include "namer.h"

branch::branch(char *name)
{
  this->name = new char[strlen(name) + 1];
  strcpy(this->name, name);
}

branch::~branch()
{
  List <branch *>*entry, *n;
  delete name;

  list_for_each_safe(entry, n, sub) {
    delete entry->item;
    delete entry;
  }

  delete sub;
}

branch *branch::add(char *name)
{
  branch *object = new branch(name);

  if (sub)
    sub->add_tail(object);
  else
    sub = new List <branch *>(object);

  return object;
}

branch *branch::find(char *name)
{
  if (!sub)
    return 0;

  List <branch *> *entry = sub;
  branch *object;

  /* пытаемся найти объект */
  do {
    object = entry->item;
    if (!strcmp(object->name, name))
      return object;
    entry = entry->next;
  } while (entry != sub);

  return 0;
}
