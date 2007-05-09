/*
  include/fs.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef __FS_H
#define __FS_H

#include <types.h>

struct file_info_t {
  u32_t type;			/* тип объекта (regular file/directory/other object) */
  u32_t nlink;			/* количество ссылок */

  uid_t uid;			/* id владельца */
  gid_t gid;			/* gid владельца */
  mode_t mode;			/* права доступа */

  size_t size;			/* размер (в байтах) */
  u32_t atime;			/* время последнего доступа */
  u32_t mtime;			/* время последней моификации */
};

#define FTypeObject    1
#define FTypeDirectory 2
//#define FTypeObject    3

#define MAX_PATH_LEN 1024
#define MAX_NAME_LEN 128

void init_fs();

//#define SEEK_SET 0
//#define SEEK_CUR 1
//#define SEEK_END 2

#endif
