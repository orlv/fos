/*
  kernel/drivers/fs/modulefs.cpp
  Copyright (C) 2006-2007 Oleg Fedorov
                     2008 Sergey Gridassov
*/

#include "modulefs.h"
#include <fos/printk.h>
#include <fos/mm.h>
#include <fos/procman.h>
#include <fos/fos.h>
#include <sys/stat.h>
#include <string.h>

#include <fos/nsi.h>

#define MODULEFS_BUFF_SIZE 0x2000

static char *buffer;
extern ModuleFS *initrb;
static struct stat *statbuf;
static struct dirent *dent;

int access(struct message *msg)
{
  buffer[msg->recv_size] = 0;
  //printk("modulefs: access to [%s]\n", buffer);
  msg->arg[0] = initrb->access(buffer) + 1;
  msg->arg[1] = MODULEFS_BUFF_SIZE;
  msg->arg[2] = NO_ERR;
  if(msg->arg[0])
    msg->arg[3] = initrb->size(msg->arg[0] - 1);
  //printk("[a0=%d]", msg->arg[0]);
  msg->send_size = 0;
  return 1;
}

int read(struct message *msg)
{
  //printk("modulefs: reading %d, offset=%d, bytes=%d\n", msg->arg[1]-1, msg->arg[2], msg->send_size);
  size_t size = msg->send_size;
  if(size > MODULEFS_BUFF_SIZE)
    size = MODULEFS_BUFF_SIZE;
  msg->arg[0] = initrb->read(msg->arg[1] - 1, msg->arg[2], buffer, size);
  if(msg->arg[0] < size)
    msg->arg[2] = ERR_EOF;
  else
    msg->arg[2] = NO_ERR;
  msg->arg[1] = initrb->size(msg->arg[1]) - 1;
  msg->send_size = msg->arg[0];
  msg->send_buf = buffer;
  return 1;
}

int stat(struct message *msg)
{
  buffer[msg->recv_size] = 0;
  //printk("modulefs: stat of [%s]\n", buffer);
  msg->arg[0] = msg->arg[1] = initrb->access(buffer) + 1;
  if(!msg->arg[0]) {
    msg->send_size = 0;
  }
  return 1;
}

int fstat(struct message *msg)
{
  //printk("modulefs: fstat(%d)\n", msg->arg[1]);
  initrb->stat(statbuf, msg->arg[1]-1);
  msg->arg[1] = MODULEFS_BUFF_SIZE;
  msg->arg[2] = NO_ERR;
  msg->send_size = sizeof(struct stat);
  msg->send_buf = statbuf;
  return 1;
}

int diropen(struct message *msg)
{
  buffer[msg->recv_size] = 0;
  if(strcmp(buffer, "/") && strcmp(buffer, ".") && strcmp(buffer, "/.")) {
    msg->arg[0] = 0;
    msg->arg[1] = 0;
    msg->arg[2] = ERR_NO_SUCH_FILE;
    msg->send_size = 0;
  } else {
    msg->arg[0] = 1;
    msg->arg[1] = initrb->count();
    msg->arg[2] = NO_ERR;
    msg->send_size = 0;
  }
  return 1;
}

int dirread(struct message *msg)
{
  if(initrb->get_name(buffer, MODULEFS_BUFF_SIZE, msg->arg[2]) < 0) {
    msg->arg[2] = ERR_EOF;
    msg->send_size = 0;
  } else {
    dent->d_ino = msg->arg[2];
    dent->d_reclen = strlen(buffer);
    strcpy(dent->d_name, buffer);
    msg->send_size = sizeof(struct dirent);
    msg->send_buf = dent;
    msg->arg[2] = NO_ERR;
  }
  return 1;
}

void grub_modulefs_srv()
{
  printk("modulefs: waiting for namer..\n");
  nsi_t *interface = new nsi_t("/mnt/modules");
  printk("modulefs: started\n");
  buffer = new char[MODULEFS_BUFF_SIZE];
  statbuf = new struct stat;
  dent = new struct dirent;

  interface->std.recv_buf = buffer;
  interface->std.recv_size = MODULEFS_BUFF_SIZE;

  interface->add(FS_CMD_ACCESS, &access);
  interface->add(FS_CMD_READ, &read);
  interface->add(FS_CMD_STAT, &stat);
  interface->add(FS_CMD_FSTAT, &fstat);
  interface->add(FS_CMD_DIROPEN, &diropen);
  interface->add(FS_CMD_DIRREAD, &dirread);
  
  while (1) {
    interface->wait_message();
  };
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

int ModuleFS::get_name(char *buf, int maxlen, unsigned int id) {
  if(id > mbi->mods_count - 1)
    return -1;
  strncpy(buf, __get_name((const char *)((module_t *) mbi->mods_addr)[id].string), maxlen);
  return 0;
}

int ModuleFS::count() {
  return mbi->mods_count;
}

int ModuleFS::access(const char *name)
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
//  printk("filename=%s\n", (const char *)((module_t *) mbi->mods_addr)[n].string);
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

ModuleFSFile::ModuleFSFile(u32_t n, ModuleFS * parentdir)
{
  num = n;
  parent = parentdir;
}

size_t ModuleFSFile::read(offs_t offset, void *buf, size_t count)
{
  return parent->read(num, offset, buf, count);
}
