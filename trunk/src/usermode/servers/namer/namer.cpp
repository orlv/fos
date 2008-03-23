/*
  namer/namer.cpp
  Copyright (C) 2006-2007 Oleg Fedorov

  - (Fri Jul 13 17:51:58 2007) перемещено в usermode
*/

/*
  Namer играет вспомогательную роль при определении сервера, отвечающего
  за конечный файл.

  Допустим, при получении запроса на открытие файла /tmp/foo.txt
  и смонированном на каталог /tmp сервера с pid=34, запрос модифицируется
  на /foo.txt и переадресуется данному серверу.

  Далее сервер сам решает, позволять ли доступ к файлу. Сервер не знает
  об факте переадресации, она производится прозрачно.
  reply от сервера уходит к вызвавшему приложению.

  Для открытия файлов следует пользоваться namer (иначе в случае вложенного монтирования
  будет открыт не тот файл)
*/

#include <fos/fs.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <fos/namer.h>
#include <string.h>
#include <stdio.h>
#include "namer.h"

static inline size_t p_len(const char *p)
{
  size_t i = 0;

  while (p[i] && (p[i] != '/'))
    i++;
  return i;
}

List < char *> *path_strip(const char *path)
{
  char *name;
  size_t len;
  const char *p = path;

  List < char *> *lpath = 0;

  while (1) {
    while (*p == '/')
      p++;

    if ((len = p_len(p))) {
      name = new char[len + 1];

      strncpy(name, p, len);
      if (lpath)
	lpath->add_tail(name);
      else
	lpath = new List < char *> (name);
      p += len;
    } else
      break;
  }

  return lpath;
}

Tobject::Tobject(const char *name)
{
  set_name(name);
}

Tobject::Tobject(const char *name, sid_t sid)
{
  this->sid = sid;
  set_name(name);
}

Tobject::~Tobject()
{
  List < Tobject * >*entry, *n;
  delete name;

  list_for_each_safe(entry, n, sub_objects) {
    delete entry->item;
    delete entry;
  }

  delete sub_objects;
}

void Tobject::set_name(const char *name)
{
  delete this->name;
  this->name = new char[strlen(name) + 1];

  strcpy(this->name, name);
}

Tobject *Tobject::add_sub(const char *name, sid_t sid)
{
  Tobject *object = new Tobject(name, sid);

  if (sub_objects) {
    sub_objects->add_tail(object);
  } else {
    sub_objects = new List < Tobject * >(object);
  }
  return object;
}

Tobject *Tobject::add_sub(const char *name)
{
  Tobject *object = new Tobject(name);

  if (sub_objects) {
    sub_objects->add_tail(object);
  } else {
    sub_objects = new List < Tobject * >(object);
  }
  return object;
}

Tobject *Tobject::access(const char *name)
{
  if (!sub_objects)
    return 0;

  List < Tobject * >*entry = sub_objects;
  Tobject *object;

  /* Пытаемся найти объект */
  do {
    object = entry->item;
    if (!strcmp(object->name, name)) {
      return object;
    }
    entry = entry->next;
  } while (entry != sub_objects);

  return 0;
}

Namer::Namer()
{
  rootdir = new Tobject(".");
}

/* режет строку пути на список из элементов пути */
List < char *> *path_strip(const char *path);

Tobject *Namer::resolve(char *name)
{
  if (strlen(name) == 1 && (strcmp(name, ".") || strcmp(name, "/"))) {
    return rootdir;
  }

  List < char *> *path = path_strip(name);
  List < char *> *entry = path;
  Tobject *object = rootdir;
  Tobject *obj = object;

  List < char *> *e = path;

  /* отыщем сервер */
  do {
    if (!(object = object->access(entry->item)))
      break;
    if (object->sid) {
      obj = object;
      e = entry;
    }
    entry = entry->next;
  } while (entry != path);

  name[0] = 0;

  /* создадим строку с окончанием пути (необходимо передать
     её конечному серверу) */
  if (obj->sid != rootdir->sid) {
    e = e->next;
    if (e == path)
      strcat(name, ".");
    else
      do {
	strcat(name, "/");
	strcat(name, e->item);
	e = e->next;
      } while (e != path);
  } else {
    do {
      strcat(name, "/");
      strcat(name, e->item);
      e = e->next;
    } while (e != path);
  }

  list_for_each_safe(entry, e, path) {
    delete entry->item;
    delete entry;
  }
  delete path->item;
  delete path;

  return obj;
}

/*
  добавляет запись об последнем объекте пути
  если промежуточных элементов не существует - они создаются
 */
Tobject *Namer::add(const char *name, sid_t sid)
{
  if (strlen(name) == 1 && (strcmp(name, ".") || strcmp(name, "/"))) {
    rootdir->sid = sid;
    return rootdir;
  }

  List < char *> *path = path_strip(name);
  List < char *> *entry = path;
  char *n;

  Tobject *object = rootdir;
  Tobject *obj;

  do {
    n = entry->item;
    if (!(obj = object->access(n))) {
      obj = object->add_sub(n);
    }
    object = obj;

    delete n;

    entry = entry->next;
  } while (entry != path);

  List < char *> *e;
  list_for_each_safe(entry, e, path) {
    delete entry;
  }
  delete path;

  object->sid = sid;
  return object;
}
