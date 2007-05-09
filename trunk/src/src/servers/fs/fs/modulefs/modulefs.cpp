/*
    kernel/drivers/fs/modulefs.cpp
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#include "modulefs.h"
#include <stdio.h>
#include <string.h>
#include <mm.h>

static const char *__get_name(const char *str)
{
  const char *ptr;
  ptr = str;
  while (*str) {
    if (*str == '/')
      if (*(str + 1) && *(str + 1) != '/')
	ptr = str + 1;

    str++;
  }
  return ptr;
}

ModuleFS::ModuleFS(multiboot_info_t * mbi)
{
  this->mbi = mbi;
  info.type = FTypeDirectory;
}

Tinterface *ModuleFS::access(const string name)
{
  u32_t i;
  Tinterface *file = 0;

  /* Ищем соответствующее имя файла */
  for (i = 0; i < mbi->mods_count; i++) {
    if (!strcmp(name, __get_name((const char *)((module_t *) mbi->mods_addr)[i].string))) {	/* нашли! */
      file = new ModuleFSFile(i, this);

      file->info.type = FTypeObject;
      file->info.uid = 0;
      file->info.gid = 0;
      file->info.mode = 0700;
      file->info.size =
	  ((module_t *) mbi->mods_addr)[i].mod_end -
	  ((module_t *) mbi->mods_addr)[i].mod_start;
      file->info.atime = 0;
      file->info.mtime = 0;
      break;
    }
  }

  return file;
}

size_t ModuleFS::read(u32_t n, off_t offset, void *buf, size_t count)
{
  u8_t *file = (u8_t *) ((module_t *) mbi->mods_addr)[n].mod_start;
  size_t size =
      ((module_t *) mbi->mods_addr)[n].mod_end -
      ((module_t *) mbi->mods_addr)[n].mod_start;

  if (offset > size)
    return 0;
  if ((offset + count) > size)
    count = size - offset;

  memcpy(buf, file, count);

  return count;
}

obj_info_t *ModuleFS::list(off_t offset)
{
  if (offset >= mbi->mods_count)	/* переполнение */
    return 0;

  u32_t i = offset;
  obj_info_t *dirent = new obj_info_t;
  dirent->info.type = FTypeObject;
  dirent->info.uid = 0;
  dirent->info.gid = 0;
  dirent->info.mode = 0700;
  dirent->info.size =
      ((module_t *) mbi->mods_addr)[i].mod_end -
      ((module_t *) mbi->mods_addr)[i].mod_start;
  dirent->info.atime = 0;
  dirent->info.mtime = 0;

  const char *name =
      __get_name((const char *)((module_t *) mbi->mods_addr)[i].string);
  strcpy(dirent->name, name);
  return dirent;
}

ModuleFSFile::ModuleFSFile(u32_t n, ModuleFS * parentdir)
{
  num = n;
  parent = parentdir;
}

size_t ModuleFSFile::read(offs_t offset, void *buf, size_t count)
{
  return parent->read(num, offset, buf, count);
}
