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

//#include <fos/fs.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <fos/namer.h>
#include <string.h>
#include <stdio.h>
#include "namer.h"

tree::tree()
{
  root = new branch(".");
}

static inline size_t p_len(const char *p)
{
  size_t i = 0;

  while (p[i] && (p[i] != '/'))
    i++;
  return i;
}

/* режет строку пути на список из элементов пути */
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

branch *tree::find_branch_last_match(char *name)
{
  if (strlen(name) == 1 && (strcmp(name, ".") || strcmp(name, "/"))) {
    return root;
  }

  List < char *> *path = path_strip(name);
  List < char *> *entry = path;
  branch *object = root;
  branch *obj = object;

  List < char *> *e = path;

  /* отыщем сервер */
  do {
    if (!(object = object->find(entry->item)))
      break;
    if (object->data) {
      obj = object;
      e = entry;
    }
    entry = entry->next;
  } while (entry != path);

  name[0] = 0;

  /* создадим строку с окончанием пути (необходимо передать
     её конечному серверу) */
  if (obj->data != root->data) {
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
branch *tree::add(char *name, void *data)
{
  if (strlen(name) == 1 && (strcmp(name, ".") || strcmp(name, "/"))) {
    root->data = data;
    return root;
  }

  List < char *> *path = path_strip(name);
  List < char *> *entry = path;
  char *n;

  branch *object = root;
  branch *obj;

  do {
    n = entry->item;
    if (!(obj = object->find(n)))
      obj = object->add(n);

    object = obj;

    delete n;

    entry = entry->next;
  } while (entry != path);

  List <char *> *e;
  list_for_each_safe(entry, e, path) {
    delete entry;
  }
  delete path;

  object->data = data;
  return object;
}

