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

asmlinkage int main()
{
  Namer *namer = new Namer;
  Tobject *obj;
  message *msg = new message;
  char *pathname = new char[MAX_PATH_LEN];
  char *path_tail = new char[MAX_PATH_LEN];

  while(1){
    msg->recv_size = MAX_PATH_LEN;
    msg->recv_buf  = pathname;
    msg->tid = _MSG_SENDER_ANY;
    receive(msg);
    switch(msg->arg[0]){
    case NAMER_CMD_ADD:
      //printf("namer: adding [%s]\n", pathname);
      obj = namer->add(pathname, msg->tid);

      if(obj)
	msg->arg[0] = RES_SUCCESS;
      else
	msg->arg[0] = RES_FAULT;

      msg->send_size = 0;
      reply(msg);
      break;

    case FS_CMD_ACCESS:
      //printf("namer: requested access to [%s]\n", pathname);
      obj = namer->access(pathname, path_tail);
      //printf("[0x%X]", obj);
      if(obj->sid){
	strcpy(pathname, path_tail);
	//printf("namer: access granted [%s]\n", pathname);
	msg->send_size = strlen(pathname);
	msg->send_buf = pathname;
	if(forward(msg, obj->sid) != RES_SUCCESS){
	  msg->arg[0] = 0;
	  msg->arg[2] = ERR_NO_SUCH_FILE;
	  reply(msg);
	}
      } else {
	//printf("namer: access denied\n");
	msg->send_size = 0;
	msg->arg[0] = 0;
	msg->arg[2] = ERR_NO_SUCH_FILE;
	reply(msg);
      }
      memset(path_tail, 0, MAX_PATH_LEN);
      break;

    case FS_CMD_STAT:
      obj = namer->access(pathname, path_tail);
      if(obj->sid){
	strcpy(pathname, path_tail);
	msg->send_size = strlen(pathname);
	msg->send_buf = pathname;
	if(forward(msg, obj->sid) != RES_SUCCESS){
	  msg->arg[0] = 0;
	  msg->arg[2] = ERR_NO_SUCH_FILE;
	  reply(msg);
	}
      } else {
	msg->send_size = 0;
	msg->arg[0] = 0;
	msg->arg[2] = ERR_NO_SUCH_FILE;
	reply(msg);
      }
      memset(path_tail, 0, MAX_PATH_LEN);
      break;

    case NAMER_CMD_RESOLVE:
      printf("namer: resolving [%s]\n", pathname);
      msg->send_size = msg->arg[0] = namer->resolve(pathname);
      msg->send_buf = pathname;
      reply(msg);
      break;

    default:
      msg->send_size = 0;
      msg->arg[0] = 0;
      reply(msg);
    }
  }
  return 0;
}

static inline size_t p_len(string p)
{
  size_t i = 0;
  while (p[i] && (p[i] != '/'))
    i++;
  return i;
}

List<string> * path_strip(const string path)
{
  string name;
  size_t len;
  string p = path;
  List<string> *lpath = 0;

  while(1){
    while (*p == '/') p++;
    
    if((len = p_len(p))){
      name = new char[len + 1];
      strncpy(name, p, len);
      if(lpath)
	lpath->add_tail(name);
      else
	lpath = new List<string>(name);
      p += len;
    }
    else break;
  }

  return lpath;
}

Tobject::Tobject(const string name)
{
  set_name(name);
}

Tobject::Tobject(const string name, sid_t sid)
{
  this->sid = sid;
  set_name(name);
}

Tobject::~Tobject()
{
  List<Tobject *> *entry, *n;
  delete name;
  
  list_for_each_safe(entry, n, sub_objects){
    delete entry->item;
    delete entry;
  }
  
  delete sub_objects;
}

void Tobject::set_name(const string name)
{
  delete this->name;
  this->name = new char[strlen(name) + 1];
  strcpy(this->name, name);
}

Tobject * Tobject::add_sub(const string name, sid_t sid)
{
  Tobject *object = new Tobject(name, sid);
  if(sub_objects){
    sub_objects->add_tail(object);
  } else {
    sub_objects = new List<Tobject *>(object);
  }
  return object;
}

Tobject * Tobject::add_sub(const string name)
{
  Tobject *object = new Tobject(name);
  if(sub_objects){
    sub_objects->add_tail(object);
  } else {
    sub_objects = new List<Tobject *>(object);
  }
  return object;
}

Tobject * Tobject::access(const string name)
{
  if(!sub_objects)
    return 0;

  List<Tobject *> *entry = sub_objects;
  Tobject * object;

  /* Пытаемся найти объект */
  do {
    object = entry->item;
    if(!strcmp(object->name, name)) {
      return object;
    }
    entry = entry->next;
  } while (entry != sub_objects);
  
  return 0;
}

Namer::Namer()
{
  rootdir = new Tobject("/");
}

/* режет строку пути на список из элементов пути */
List<string> * path_strip(const string path);

Tobject * Namer::access(const string name, string name_tail)
{
  List<string> *path = path_strip(name);
  List<string> *entry = path;
  Tobject * object = rootdir;
  Tobject * obj = object;
  List<string> *e = path;

  /* отыщем сервер */
  do {
    if(!(object = object->access(entry->item)))
      break;
    if(object->sid){
      obj = object;
      e = entry;
    }
    entry = entry->next;
  } while (entry != path);
  
  /* создадим строку с окончанием пути (необходимо передать
     её конечному серверу) */
  e = e->next;
  if (e == path)
    strcat(name_tail, ".");
  else
    do {
      strcat(name_tail, "/");
      strcat(name_tail, e->item);
      e = e->next;
    } while (e != path);

  list_for_each_safe(entry, e, path){
    delete entry->item;
    delete entry;
  }
  delete path->item;
  delete path;

  return obj;
}

sid_t Namer::resolve(const string name)
{
  List<string> *path = path_strip(name);
  List<string> *entry = path;
  List<string> *e;
  Tobject * object = rootdir;
  sid_t sid = rootdir->sid;

  /* отыщем сервер */
  do {
    if(!(object = object->access(entry->item)))
      break;
    if(object->sid) {
      sid = object->sid;
      e = entry;
    }
    entry = entry->next;
  } while (entry != path);

  /* создадим строку с окончанием пути (необходимо передать
     её конечному серверу) */
  e = e->next;
  name[0] = 0;
  if (e == path)
    strcat(name, ".");
  else
    do {
      strcat(name, "/");
      strcat(name, e->item);
      e = e->next;
    } while (e != path);

  list_for_each_safe(entry, e, path){
    delete entry->item;
    delete entry;
  }
  delete entry->item;
  delete entry;

  return sid;
}

/*
  добавляет запись об последнем объекте пути
  если промежуточных элементов не существует - они создаются
 */
Tobject *Namer::add(const string name, sid_t sid)
{
  List<string> *path = path_strip(name);
  List<string> *entry = path;
  string n;

  Tobject * object = rootdir;
  Tobject * obj;

  do {
    n = entry->item;
    if(!(obj = object->access(n))){
      obj = object->add_sub(n);
    }
    object = obj;

    delete n;
    
    entry = entry->next;
  } while (entry != path);

  List<string> *e;
  list_for_each_safe(entry, e, path){
    delete entry;
  }
  delete path;

  object->sid = sid;
  return object;
}
