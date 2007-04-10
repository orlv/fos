/*
  include/fslayer.h
  Copyright (C) 2006 Oleg Fedorov

  2006-11-12. Всё хорошенько переделано. Oleg.
*/

#ifndef __FSLAYER_H
#define __FSLAYER_H

#include <types.h>
#include <tinterface.h>
#include <list.h>

/* Cache Object */
class Tobject {
public:
  Tobject(Tinterface * interface, const string name);
   Tobject(class Tdirectory * directory, const string name);

  ~Tobject();

  Tinterface *object;
  class Tdirectory *directory;

  List *mnt_list;

  void mount(Tdirectory * container);
  res_t umount(Tcontainer * container);

  string name;
};

/* Layer Directory */
class Tdirectory {
private:
  List * objlist;
  Tobject *insert2cache(Tinterface * interface, const string name);

public:
   Tdirectory(Tcontainer * container, Tdirectory * parent);
  ~Tdirectory();

  /* Связанный объект-директория файловой системы */
  Tcontainer *container;

  /* Прочитать запись об одном объекте в ФС */
  obj_info_t *list(off_t offset);

  /*
     Получить доступ к объекту (аналог open()).
     Если объект ещё отсутствует в кэше, он загружается.
   */
  Tobject *access(const string name);

  Tobject *mount(Tcontainer * container, const string name);
  res_t umount(const string name, Tcontainer * container);

  /*
     Создать стандартный для этой ФС объект.
     type - обычно файл или директория.
   */
  Tobject *construct(const string name, u32_t type, Tinterface * interface);

  /* Удалить объект из ФС и кэша */
  res_t remove(const string name);
};

#endif
