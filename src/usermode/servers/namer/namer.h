/*
  namer/namer.h
  Copyright (C) 2006-2007 Oleg Fedorov

  2006-11-12. (fslayer.h) Всё хорошенько переделано. Oleg.
  (Fri May 25 22:11:24 2007) использовано при создании namer
*/

#ifndef __NAMER_H
#define __NAMER_H

#include <types.h>
#include <c++/list.h>

class Tobject{
 public:
  Tobject(const string name, sid_t sid);
  Tobject(const string name);
  ~Tobject();

  sid_t sid;
  string name;
  List<Tobject *> *sub_objects; /* объект может содержать в себе другие объекты */

  void set_name(const string name); /* меняет имя объекта */

  Tobject * add_sub(const string name, sid_t sid);
  Tobject * add_sub(const string name);

  Tobject * access(const string name);
};

class Namer {
 public:
  Tobject *rootdir;
  Namer();

  Tobject * resolve(const string name);
  Tobject * add(const string name, sid_t sid);
  
  //res_t remove(const string name);
  //obj_info_t *list(off_t offset);  
};

#endif
