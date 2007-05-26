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

#define MAX_PATH_LEN 4096
#define MAX_NAME_LEN 256

#define FS_CMD_ACCESS 0
#define FS_CMD_READ   1
#define FS_CMD_WRITE  2
#define FS_CMD_LIST   3

#define NAMER_CMD_ACCESS 0
#define NAMER_CMD_ADD    4
#define NAMER_CMD_REM    5

#define PID_NAMER 2

struct fs_message {
  u32_t cmd;
  char buf[MAX_PATH_LEN];
} __attribute__ ((packed));

#endif
