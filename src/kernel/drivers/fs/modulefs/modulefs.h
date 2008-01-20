/*
  modulefs.h
  Copyright (C) 2006-2007 Oleg Fedorov
                     2008 Sergey Gridassov
*/

#ifndef __MODULEFS_H
#define __MODULEFS_H

#include <types.h>
#include <multiboot.h>

class ModuleFS {
 private:
  multiboot_info_t * mbi;

 public:
  ModuleFS(multiboot_info_t * mbi);
 
  size_t read(u32_t n, off_t offset, void *buf, size_t count);
  int access(const string name);
  void stat(struct stat *statbuf, u32_t n);
  size_t size(u32_t n);
  int count();
  int get_name(char *buf, int maxlen, unsigned int id);
};

class ModuleFSFile {
 private:
  ModuleFS * parent;
  u32_t num;

 public:
  ModuleFSFile(u32_t n, ModuleFS * parent);
  size_t read(off_t offset, void *buf, size_t count);
};

#endif
