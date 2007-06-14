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
#include <system.h>

void namer_srv()
{
  printk("[Namer]\n");

  hal->namer = new Namer;

  Tobject *obj;
  message *msg = new message;
  fs_message *m = new fs_message;
  u32_t res;

  hal->namer->add("/sys/namer", (sid_t)hal->ProcMan->CurrentThread);
  
  while(1){
    asm("incb 0xb8000+152\n" "movb $0x1f,0xb8000+153 ");
    msg->recv_size = sizeof(fs_message);
    msg->recv_buf = m;

    receive(msg);
  
    switch(m->data.cmd){
    case NAMER_CMD_ADD:
      obj = hal->namer->add(m->data3.buf, msg->tid);

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
      obj = hal->namer->access(m->data3.buf, m->data3.buf);
      
      if(obj){
	printk("[%s]", m->data3.buf);
      }
      else
	res = 0;
      
      forward(msg, obj->sid);
      break;
      
    case NAMER_CMD_RESOLVE:
      obj = hal->namer->access(m->data3.buf, 0);
      res = obj->sid;
      msg->recv_size = 0;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      reply(msg);
      break;

    case NAMER_CMD_REM:
    default:
      res = RES_FAULT;
      msg->recv_size = 0;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      reply(msg);
    }
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

Namer::Namer()
{
  rootdir = new Tobject("/");
}

/* режет строку пути на список из элементов пути */
List * path_strip(const string path);

Tobject * Namer::access(const string name, string t_name)
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
  if(t_name){
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
Tobject *Namer::add(const string name, sid_t sid)
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
