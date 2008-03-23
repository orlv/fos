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



class branch {
 public:
  char *name;
  void *data;
  List <branch *> *sub;

  branch(char *name);
  ~branch();
  branch * add(char *name);
  branch * find(char *name);
};

class tree {
 public:
  branch *root;
  tree();

  branch* add(char *name, void *data);
  void remove(char *name);

  branch * find_branch(char *name);
  branch * find_branch_last_match(char *name);
};

#endif
