/*
   kernel/main/fs/objectfs.cpp
   Copyright (C) 2006 Oleg Fedorov
*/

#include "objectfs.h"
#include <string.h>
#include <stdio.h>

ObjectFS_object::ObjectFS_object(Tinterface * interface, const string name)
{
  if (interface) {
    this->interface = interface;
    interface->info.nlink++;
  }

  this->name = new char[strlen(name) + 1];
  strcpy(this->name, name);
}

ObjectFS_object::~ObjectFS_object()
{
  /* TODO: больше проверок */
  delete name;
  interface->info.nlink--;
  if (!interface->info.nlink)
    delete interface;
}

ObjectFS::ObjectFS(Tcontainer * parent):Tcontainer()
{
  info.type = FTypeDirectory;
  if (!parent)
    parent = this;

  objlist = new List(new ObjectFS_object(this, "."));
  objlist->add_tail(new ObjectFS_object(parent, ".."));
}

ObjectFS::~ObjectFS()
{
  /*
     Удалим список объектов.
     ВНИМАНИЕ! Все интерфейсы, если не нужны, должны быть удалены заранее!
   */
  delete objlist;
}

/*
  Ищем запись с именем, соответствующим name.
  В случае удачи возвращаем соответствующий interface 
*/
Tinterface *ObjectFS::access(const string name)
{
  List *entry = objlist;
  ObjectFS_object *object;
  do {
    object = (ObjectFS_object *) entry->data;
    if (!strcmp(object->name, name)) {
      return object->interface;	// найден
    }
    entry = entry->next;
  } while (entry);
  return 0;			// не найден
}

/*
  Ищем в директории запись с указанным именем.
  Если находим - удаляем.
*/
res_t ObjectFS::remove(const string name)
{
  List *entry = objlist;
  ObjectFS_object *object;

  do {
    object = (ObjectFS_object *) entry->data;
    if (!strcmp(object->name, name)) {
      delete object->interface;
      delete entry;
      return RES_SUCCESS;
    }
    entry = entry->next;
  } while (entry != objlist);
  return RES_FAULT;
}

/* Читает одну запись из директории */
obj_info_t *ObjectFS::list(off_t offset)
{
  List *entry = objlist;
  off_t n = 0;
  /*
     Отсчитаем offset записей. По окончанию поиска dirent будет 
     указывать на нужную запись
   */

#warning ** ObjectFS::list() : TODO: оптимизировать! ******

  do {
    entry = entry->next;
    n++;
  } while ((n < offset) && (entry != objlist));

  if ((n != offset) && (entry == objlist))
    return 0;

  ObjectFS_object *object = (ObjectFS_object *) entry->data;
  obj_info_t *dirent = new obj_info_t;

  dirent->info.type = object->interface->info.type;
  dirent->info.nlink = object->interface->info.nlink;
  dirent->info.uid = object->interface->info.uid;
  dirent->info.gid = object->interface->info.gid;
  dirent->info.mode = object->interface->info.mode;
  dirent->info.size = object->interface->info.size;
  dirent->info.atime = object->interface->info.atime;
  dirent->info.mtime = object->interface->info.mtime;

  strcpy(dirent->name, object->name);
  return dirent;
}

Tinterface *ObjectFS::construct(const string name, u32_t type,
				Tinterface * interface)
{
  Tinterface *i = 0;
  /* 
     type = FTypeObject    1
     type = FTypeDirectory 2
   */
  if (type == FTypeObject) {	// просто сохраняем полученный интерфейс (драйвер полноценной ФС вместо этого должен создать свой объект, игнорируя interface)
    i = interface;
  } else if (type == FTypeDirectory) {	// создаём подкаталог
    i = new ObjectFS(this);
  }

  if (i) {
    i->info.nlink++;
    objlist->add_tail(new ObjectFS_object(i, name));
  }

  return i;
}
