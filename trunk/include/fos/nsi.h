/*
  include/fos/nsi.h
  Copyright (C) 2008 Oleg Fedorov

  Named Server Interface's framework for FOS
 */

#ifndef _FOS_NSI_H
#define _FOS_NSI_H

#if 0
server srv = new server("mntpoint");
object obj = srv->add("test.txt");
obj->add_metod(READ, &read);
/* ..... */
obj = srv->resolve("test.txt");
obj->call(READ, args);


namer namer = new namer();
object obj = namer->add("/dev/srv", tid);
/* ..... */
obj = namer->resolve("/dev/srv");
obj->forward(msg);
#endif


#include <types.h>
#include <c++/list.h>
#include <fos/fs.h>

class nsi_object {
 public:
  char *name;
  u32_t methods[MAX_METHODS_CNT];

  inline bool add(u32_t method, u32_t function){
    if(method < MAX_METHODS_CNT){
      methods[method] = function;
      return 0;
    }
    return 1;
  }
  inline void remove(u32_t method){
    if(method < MAX_METHODS_CNT)
      methods[method] = 0;
  }
  inline void call(u32_t method){
    
  }
};

#if 0
class nsi_t {
 private:
  List <nsi_object *> *objects;
  nsi_object * find(char *name);

 public:
  nsi_t(char *bindpath);

  nsi_object * add(char *name);
  void remove(char *name);
};
#endif

#endif
