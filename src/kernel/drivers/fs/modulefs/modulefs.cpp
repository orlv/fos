/*
  kernel/drivers/fs/modulefs.cpp
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#include "modulefs.h"
#include <fos/printk.h>
#include <fos/mm.h>
#include <fos/procman.h>
#include <sys/stat.h>
#include <string.h>

#define MODULEFS_BUFF_SIZE 0x2000

void grub_modulefs_srv()
{
  struct message msg;
  char *buffer = new char[MODULEFS_BUFF_SIZE];
  extern ModuleFS *initrb;
  printk("modulefs: waiting for namer..\n");
  while(resmgr_attach("/mnt/modules") != RES_SUCCESS);
  printk("modulefs: started\n");
  struct stat *statbuf = new struct stat;

  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_buf  = buffer;
    msg.recv_size = MODULEFS_BUFF_SIZE;
    receive(&msg);

    switch(msg.a0){
    case FS_CMD_ACCESS:
      buffer[msg.recv_size] = 0;
      //printk("modulefs: access to [%s]\n", buffer);
      msg.a0 = initrb->access(buffer) + 1;
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

    case FS_CMD_STAT:
      buffer[msg.recv_size] = 0;
      //printk("modulefs: stat of [%s]\n", buffer);
      msg.a0 = msg.a1 = initrb->access(buffer) + 1;
      if(!msg.a0) {
	msg.send_size = 0;
	break;
      }

    case FS_CMD_FSTAT:
      //printk("modulefs: fstat(%d)\n", msg.a1);
      initrb->stat(statbuf, msg.a1-1);
      msg.a1 = MODULEFS_BUFF_SIZE;
      msg.a2 = NO_ERR;
      msg.send_size = sizeof(struct stat);
      msg.send_buf = statbuf;
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
}

int ModuleFS::access(const string name)
{
  char *_name = (char *) __get_name(name);
  /* Ищем соответствующее имя файла */
  for (u32_t i = 0; i < mbi->mods_count; i++) {
    if (!strcmp(_name, __get_name((const char *)((module_t *) mbi->mods_addr)[i].string))) /* нашли! */
      return i;
  }

  return -1;
}

void ModuleFS::stat(struct stat *statbuf, u32_t n)
{
  statbuf->st_dev     = 0;
  statbuf->st_ino     = n;
  statbuf->st_mode    = 0777;
  statbuf->st_nlink   = 1;
  statbuf->st_uid     = 0;
  statbuf->st_gid     = 0;
  statbuf->st_rdev    = 0;
      
  statbuf->st_size    = size(n);
      
  statbuf->st_blksize = 1;
  statbuf->st_blocks  = statbuf->st_size;
  statbuf->st_atime   = 0;
  statbuf->st_mtime   = 0;
  statbuf->st_ctime   = 0;
}

size_t ModuleFS::size(u32_t n)
{
  return ((module_t *) mbi->mods_addr)[n].mod_end -
    ((module_t *) mbi->mods_addr)[n].mod_start;
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

#if 0
obj_info_t *ModuleFS::list(off_t offset)
{
  if (offset >= mbi->mods_count)	/* переполнение */
    return 0;

  u32_t i = offset;
  obj_info_t *dirent = new obj_info_t;
  dirent->info.type = FTypeObject;
  dirent->info.uid = 0;
  dirent->info.gid = 0;
  dirent->info.mode = 0777;
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
#endif

ModuleFSFile::ModuleFSFile(u32_t n, ModuleFS * parentdir)
{
  num = n;
  parent = parentdir;
}

size_t ModuleFSFile::read(offs_t offset, void *buf, size_t count)
{
  return parent->read(num, offset, buf, count);
}
