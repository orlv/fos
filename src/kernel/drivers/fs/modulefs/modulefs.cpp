/*
  kernel/drivers/fs/modulefs.cpp
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#include "modulefs.h"
#include <stdio.h>
#include <string.h>
#include <mm.h>
#include <procman.h>

#define MODULEFS_BUFF_SIZE 0x2000

void grub_modulefs_srv()
{
  struct message msg;
  char *buffer = new char[MODULEFS_BUFF_SIZE];
  extern ModuleFS *initrb;
  while(resmgr_attach("/mnt/modules") != RES_SUCCESS);
  printk("modulefs server started\n");
  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_buf  = buffer;
    msg.recv_size = MODULEFS_BUFF_SIZE;
    receive(&msg);

    switch(msg.a0){
    case FS_CMD_ACCESS:
      buffer[msg.recv_size] = 0;
      //printk("modulefs: access to [%s]\n", buffer);
      msg.a0 = initrb->access2(buffer) + 1;
      msg.a1 = MODULEFS_BUFF_SIZE;
      msg.a2 = NO_ERR;
      //printk("[a0=%d]", msg.a0);
      msg.send_size = 0;
      break;

    case FS_CMD_READ:
      //printk("modulefs: reading %d, offset=%d, bytes=%d\n", msg.a1-1, msg.a2, msg.send_size);
      msg.a0 = initrb->read(msg.a1 - 1, msg.a2, buffer, msg.send_size);
      if(msg.a0 < msg.send_size) {
	msg.send_size = msg.a0;
	msg.a2 = ERR_EOF;
      } else
	msg.a2 = NO_ERR;
      msg.send_buf = buffer;
      break;

    default:
      msg.a0 = 0;
      msg.a2 = ERR_UNKNOWN_CMD;
      msg.send_size = 0;
    }
    reply(&msg);
  }
}

static const char *__get_name(const char *str)
{
  const char *ptr;
  ptr = str;
  while (*str) {
    str++;
    if (*(str-1) == '/')
      if (*str && *str != '/')
	ptr = str;
  }
  return ptr;
}

ModuleFS::ModuleFS(multiboot_info_t * mbi)
{
  this->mbi = mbi;
  info.type = FTypeDirectory;
}

int ModuleFS::access2(const string name)
{
  int i;
  char *_name = (char *) __get_name(name);
  /* Ищем соответствующее имя файла */
  for (i = 0; i < (int) mbi->mods_count; i++) {
    if (!strcmp(_name, __get_name((const char *)((module_t *) mbi->mods_addr)[i].string))) /* нашли! */
      return i;
  }

  return -1;
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

  memcpy(buf, &file[offset], count);

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
