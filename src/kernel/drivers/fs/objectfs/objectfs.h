/*
  include/drivers/fs/ramfs.h
  Copyright (C) 2006 Oleg Fedorov

  Основана на FS Layer.
  Не требует объекта RamFS_file и своего распределителя памяти.
*/

#ifndef __OBJECTFS_H
#define __OBJECTFS_H

#include <types.h>
#include <tinterface.h>
#include <list.h>

/* Элемент директории */
class ObjectFS_object {
public:
  ObjectFS_object(Tinterface * interface, const string name);
  ~ObjectFS_object();

  Tinterface *interface;
  string name;
};

class ObjectFS:public Tcontainer {
private:
  List * objlist;

public:
  ObjectFS(Tcontainer * parent);
  ~ObjectFS();

  /* Прочитать запись об одном объекте */
  obj_info_t *list(off_t offset);

  /* Получить доступ непосредственно к объекту (аналог open()) */
  Tinterface *access(const string name);

  /*
     Создать стандартный для этой ФС объект.
     type - обычно файл или директория.
   */
  Tinterface *construct(const string name, u32_t type, Tinterface * interface);

  /* Удалить объект из ФС */
  res_t remove(const string name);
};

#endif
