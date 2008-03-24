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
#include <string.h>

class nsi_object {
 public:
  char *name;
  void (*method [MAX_METHODS_CNT]) (struct message *msg);

  nsi_object() { }
  
  nsi_object(char *name) {
    if(name) {
      this->name = strdup(name);
    }
  }
  
  inline bool add(u32_t n,   void (*method) (struct message *msg)){
    if(n < MAX_METHODS_CNT){
      this->method[n] = method;
      return 0;
    }
    return 1;
  }
  inline void remove(u32_t n){
    if(n < MAX_METHODS_CNT)
      method[n] = 0;
  }
  inline void call(u32_t n, message *msg){
    if(n < MAX_METHODS_CNT && method[n])
      method[n](msg);
  }
};

class nsi_t {
 private:
  List <nsi_object *> *objects;
  nsi_object * find(char *name);
  message *msg;

 public:
  nsi_object root;
  
  nsi_t(char *bindpath);

  nsi_object * add(char *name);
  void remove(char *name);
  void wait_message();
  void wait_message(u32_t timeout);
};


#endif
