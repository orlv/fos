/*
  kernel/main/fs/fs.cpp
  Copyright (C) 2006-2007 Oleg Fedorov
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

#include <hal.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>
#include <namer.h>
//#include <drivers/fs/objectfs/objectfs.h>
#include <system.h>

#if 0
static inline void ls(Tdirectory * dir)
{
  obj_info_t *dirent;
  off_t i;
  for (i = 0; (dirent = dir->list(i)); i++) {
    if (dirent->info.type == FTypeDirectory)
      printf("drwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);
    else
      printf("-rwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);

    delete dirent;
  }
}
#endif

void namer_srv()
{
  printk("[Namer]\n");

  namer *Namer = new namer;

  Tobject *obj;
  struct message *msg = new struct message;
  struct fs_message *m = new fs_message;
  u32_t res;

  while(1){
    msg->recv_size = sizeof(fs_message);
    msg->recv_buf = m;
    receive(msg);
    printk("Namer: cmd=%d, string=\"%s\"\n", m->cmd, m->buf);
    
    switch(m->cmd){
    case NAMER_CMD_ADD:
      obj = Namer->add(m->buf, msg->pid);

      if(obj)
	res = RES_SUCCESS;
      else
	res = RES_FAULT;

      msg->recv_size = 0;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      reply(msg);
      break;

    case NAMER_CMD_ACCESS:
      obj = Namer->access(m->buf, m->buf);
      
      if(obj){
	printk("[%s]", m->buf);
      }
      else
	res = 0;
      
      forward(msg, obj->sid);
      break;
      
    case NAMER_CMD_REM:
    default:
      res = RES_FAULT;
      msg->recv_size = 0;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      reply(msg);
    }
    
    //forward(msg, 4);  
  }
}

Tobject::Tobject(const string name, sid_t sid)
{
  this->sid = sid;
  set_name(name);
}

Tobject::Tobject(const string name)
{
  set_name(name);
}

void Tobject::set_name(const string name)
{
  delete this->name;
  this->name = new char[strlen(name) + 1];
  strcpy(this->name, name);
}

Tobject::~Tobject()
{
  List *entry, *n;
  delete name;
  
  list_for_each_safe(entry, n, sub_objects){
    delete (Tobject *) entry->data;
    delete entry;
  }
  
  delete sub_objects;
}

Tobject * Tobject::add_sub(const string name, sid_t sid)
{
  Tobject *object = new Tobject(name, sid);
  if(sub_objects){
    sub_objects->add_tail(object);
  } else {
    sub_objects = new List(object);
  }
  return object;
}

Tobject * Tobject::add_sub(const string name)
{
  Tobject *object = new Tobject(name);
  if(sub_objects){
    sub_objects->add_tail(object);
  } else {
    sub_objects = new List(object);
  }
  return object;
}

Tobject * Tobject::access(const string name)
{
  if(!sub_objects)
    return 0;

  List *entry = sub_objects;
  Tobject * object;

  /* Пытаемся найти объект */
  do {
    object = (Tobject *) entry->data;
    if(!strcmp(object->name, name)) {
      return object;
    }
    entry = entry->next;
  } while (entry != sub_objects);
  
  return 0;
}

namer::namer()
{
  rootdir = new Tobject("/");
}

/* режет строку пути на список из элементов пути */
List * path_strip(const string path);

Tobject * namer::access(const string name, string t_name)
{
  List *path = path_strip(name);
  List *entry = path;
  string n;
  Tobject * object = rootdir;
  Tobject * o = object;
  List *e;
  
  //size_t len = strlen(name);
  //  string t_name = new char[len+1];
  //printk("Namer: %d:%s\n",len, name);
  int i=0, i1=0;

  /* отыщем сервер */
  do {
    n = (string)entry->data;
    if(!(object = object->access(n))){
      break;
    }

    if(object->sid){
      o = object;
      i = i1;
    }

    i1++;
    entry = entry->next;
  } while (entry != path);

  /* создадим переменную с окончанием пути (необходимо передать
     её конечному серверу) */
  t_name[0] = 0;
  list_for_each(entry, path){
    if(i)
      i--;
    else {
      n = (string)entry->data;
      strcat(t_name, "/");
      strcat(t_name, n);
    }
  }

  list_for_each_safe(entry, e, path){
    delete (string)entry->data;
    delete entry;
  }
  delete path;

  return o;
}

/*
  добавляет запись об последнем объекте пути
  если промежуточных элементов не существует - они создаются
 */
Tobject *namer::add(const string name, sid_t sid)
{
  List *path = path_strip(name);
  List *entry = path;
  string n;

  Tobject * object = rootdir;
  Tobject * o;

  do {
    n = (string)entry->data;
    if(!(o = object->access(n))){
      o = object->add_sub(n);
    }
    object = o;

    delete n;
    
    entry = entry->next;
  } while (entry != path);

  List *e;
  list_for_each_safe(entry, e, path){
    delete entry;
  }
  delete path;

  object->sid = sid;
  return object;
}
