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

int close(fd_t fd)
{
  if(!fd)
    return -1;

  delete fd;
  return 0;
}

size_t read(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  msg.a0 = FS_CMD_READ;
  msg.recv_size = count;
  msg.recv_buf = buf;
  msg.send_size = 0;
  msg.a1 = fd->id;
  msg.a2 = fd->offset;
  msg.tid = fd->thread;

  do{
    switch(send(&msg)){
    case RES_SUCCESS:
      return msg.recv_size;
      
    case RES_FAULT2: /* очередь получателя переполнена, обратимся чуть позже */
      continue;
      
    default:
      return 0;
    }
  }while(1);
}

size_t write(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  msg.a0 = FS_CMD_WRITE;
  msg.recv_size = 0;
  msg.send_buf = buf;
  msg.a1 = fd->id;
  msg.a2 = fd->offset;
  msg.tid = fd->thread;

  do{
    msg.send_size = count;
    
    switch(send(&msg)){
    case RES_SUCCESS:
      return msg.a0;
      
    case RES_FAULT2: /* очередь получателя переполнена, обратимся чуть позже */
      continue;
      
    default:
      return 0;
    }
  }while(1);
}

fd_t open(const char *pathname, int flags)
{
  volatile struct message msg;
  msg.a0 = FS_CMD_ACCESS;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((message *)&msg);
  if(result == RES_SUCCESS && msg.a0) {
    struct fd *fd = new struct fd;
    fd->thread = msg.tid;
    fd->id = msg.a0;
    return fd;
  } else
    return 0;
}

int resmgr_attach(const char *pathname)
{
  if(!pathname)
    return 0;

  message msg;
  msg.a0 = NAMER_CMD_ADD;
  size_t len = strlen(pathname);
  if(len+1 > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.tid = SYSTID_NAMER;
  return send((message *)&msg);
}

void namer_srv()
{
  Namer *namer = new Namer;
  hal->namer = namer;
  Tobject *obj;
  message *msg = new message;
  char *pathname = new char[MAX_PATH_LEN];
  char *path_tail = new char[MAX_PATH_LEN];

  hal->namer->add("/sys/namer", (sid_t)hal->procman->CurrentThread);
  printk("namer: ready\n");
  while(1){
    msg->recv_size = MAX_PATH_LEN;
    msg->recv_buf  = pathname;

    receive(msg);
    //printk("namer: a0=%d from [%s]\n", msg->a0, THREAD(msg->tid)->process->name);
    switch(msg->a0){
    case NAMER_CMD_ADD:
      //printk("namer: adding [%s]\n", pathname);
      obj = hal->namer->add(pathname, msg->tid);

      if(obj)
	msg->a0 = RES_SUCCESS;
      else
	msg->a0 = RES_FAULT;

      msg->send_size = 0;
      reply(msg);
      break;

    case FS_CMD_ACCESS:
      //printk("namer: requested access to [%s]\n", pathname);
      obj = hal->namer->access(pathname, path_tail);
      //printk("[0x%X]", obj);
      if(obj->sid){
	strcpy(pathname, path_tail);
	//printk("namer: access granted [%s]\n", pathname);
	msg->send_size = strlen(pathname);
	msg->send_buf = pathname;
	forward(msg, obj->sid);
      } else {
	//printk("namer: access denied\n");
	msg->send_size = 0;
	msg->a0 = 0;
	reply(msg);
      }
      memset(path_tail, 0, MAX_PATH_LEN);
      break;

      //case NAMER_CMD_RESOLVE:
      //printk("namer: resolving [%s]\n", m->data3.buf);
      /*      msg->a0 = hal->namer->resolve(pathname);
      msg->send_size = 0;
      reply(msg);
      break;*/

      //case NAMER_CMD_REM:
    default:
      msg->send_size = 0;
      msg->a0 = 0;
      reply(msg);
    }
  }
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
  do {
    strcat(name_tail, "/");
    strcat(name_tail, e->item);
    e = e->next;
  } while (e != path);

  list_for_each_safe(entry, e, path){
    delete entry->item;
    delete entry;
  }
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

    if(object->sid)
      sid = object->sid;

    entry = entry->next;
  } while (entry != path);

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
