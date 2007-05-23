/*
  tinterface.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _TINTERFACE_H
#define _TINTERFACE_H

#include <types.h>
#include <fs.h>
#include "fs.h"

struct obj_info_t {
  file_info_t info;
  size_t namelen;
  char name[MAX_NAME_LEN];
};

class Tinterface {
public:
  Tinterface();
  virtual ~ Tinterface();

  class Tcontainer *container;

  virtual size_t read(off_t offset, void *buf, size_t count);
  virtual size_t write(off_t offset, const void *buf, size_t count);

  file_info_t info;

  Tinterface *stdout;
  Tinterface *stdin;
};

class Tcontainer:public Tinterface {
public:
  Tcontainer();
  virtual ~ Tcontainer();

  /* Прочитать запись об одном объекте */
  virtual obj_info_t *list(off_t offset);

  /* Получить доступ к объекту (аналог open()) */
  virtual Tinterface *access(const string name);

  /* Создать стандартный для этой ФС объект.
     type - обычно файл или директория */
  virtual Tinterface *construct(const string name, u32_t type,
				Tinterface * interface);

  /* Удалить объект из ФС */
  virtual res_t remove(const string name);
};

#endif
