/*
  modulefs.h
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#ifndef __MODULEFS_H
#define __MODULEFS_H

#include <types.h>
#include <tinterface.h>
#include <multiboot.h>

class ModuleFS:public Tcontainer {
private:
  multiboot_info_t * mbi;

public:
  ModuleFS(multiboot_info_t * mbi);

  obj_info_t *list(off_t offset);
  size_t read(u32_t n, off_t offset, void *buf, size_t count);
  Tinterface *access(const string name);
};

class ModuleFSFile:public Tinterface {
private:
  ModuleFS * parent;
  u32_t num;

public:
   ModuleFSFile(u32_t n, ModuleFS * parent);
  size_t read(off_t offset, void *buf, size_t count);
};

#endif
