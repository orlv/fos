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

tid_t resolve(char *name)
{
  while(!hal->namer);
  volatile struct message msg;
  u32_t res;
  union fs_message m;
  msg.recv_size = sizeof(res);
  msg.recv_buf = &res;
  msg.send_size = sizeof(fs_message);
  m.data3.cmd = NAMER_CMD_RESOLVE;
  strcpy((char *)m.data3.buf, name);
  msg.send_buf = (char *)&m;
  msg.tid = 0;
  send((message *)&msg);
  return res;
}

void namer_add(string name)
{
  while(!hal->tid_namer);
  struct message *msg = new struct message;
  u32_t res;
  union fs_message *m = new fs_message;
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(union fs_message);
  msg->send_buf = m;
  strcpy(m->data3.buf,  name);
  m->data3.cmd = NAMER_CMD_ADD;
  msg->tid = 0;
  send(msg);
}

void namer_srv()
{
  Namer *namer = new Namer;
  hal->namer = namer;
  Tobject *obj;
  message *msg = new message;
  fs_message *m = new fs_message;
  u32_t res;

  hal->namer->add("/sys/namer", (sid_t)hal->ProcMan->CurrentThread);

  printk("namer: ready\n");

  while(1){
    msg->recv_size = sizeof(fs_message);
    msg->recv_buf = m;

    receive(msg);
    //printk("namer: cmd=0x%X\n", m->data.cmd);
    switch(m->data.cmd){
    case NAMER_CMD_ADD:
      //printk("adding [%s]\n", m->data3.buf);
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
      
      /*if(obj){
	printk("namer: access to [%s]\n", m->data3.buf);
	}*/
      //else
      //res = 0;
      
      forward(msg, obj->sid);
      break;
      
    case NAMER_CMD_RESOLVE:
      //printk("namer: resolving [%s]\n", m->data3.buf);
      obj = hal->namer->access(m->data3.buf, 0);
      if(obj){
	//printk("[%s]", m->data3.buf);
	res = obj->sid;
      } else {
	res = 0;
      }

      msg->recv_size = 0;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      reply(msg);
      break;

    case NAMER_CMD_REM:
    default:
      res = 0;
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
  List<Tobject *> *entry, *n;
  delete name;
  
  list_for_each_safe(entry, n, sub_objects){
    delete entry->item;
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

Tobject * Namer::access(const string name, string t_name)
{
  List<string> *path = path_strip(name);
  List<string> *entry = path;
  string n;
  Tobject * object = rootdir;
  Tobject * obj = object;
  List<string> *e;
  
  //size_t len = strlen(name);
  //  string t_name = new char[len+1];
  //printk("Namer: %d:%s\n",len, name);
  int i=0, i1=0;

  /* отыщем сервер */
  do {
    n = entry->item;
    if(!(object = object->access(n))){
      break;
    }

    if(object->sid){
      obj = object;
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
	n = entry->item;
	strcat(t_name, "/");
	strcat(t_name, n);
      }
    }
  }
  
  list_for_each_safe(entry, e, path){
    delete entry->item;
    delete entry;
  }
  delete path;

  return obj;
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
