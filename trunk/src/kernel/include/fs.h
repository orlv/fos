/*
  include/fs.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef __FS_H
#define __FS_H

#include <types.h>

struct info_t {
  u32_t type;			/* тип объекта (regular file/directory/other object) */
  u32_t nlink;			/* количество ссылок */

  uid_t uid;			/* id владельца */
  gid_t gid;			/* gid владельца */
  mode_t mode;			/* права доступа */

  size_t size;			/* размер (в байтах) */
  u32_t atime;			/* время последнего доступа */
  u32_t mtime;			/* время последней моификации */
};

struct fd {
  tid_t thread;
  offs_t offset;
  u32_t id;
};

typedef struct fd* fd_t;

#define FTypeObject    1
#define FTypeDirectory 2

#define MAX_PATH_LEN 1024
#define MAX_NAME_LEN 256
#define FS_CMD_LEN   1024

#define FS_CMD_ACCESS 0
#define FS_CMD_READ   1
#define FS_CMD_WRITE  2
#define FS_CMD_LIST   3

#define BASE_CMD_N 64

#define NAMER_CMD_ACCESS  FS_CMD_ACCESS
#define NAMER_CMD_ADD     (BASE_CMD_N + 0)
#define NAMER_CMD_REM     (BASE_CMD_N + 1)
#define NAMER_CMD_RESOLVE (BASE_CMD_N + 2)

int close(fd_t fd);
fd_t open(const char *pathname, int flags);
size_t read(fd_t fd, void *buf, size_t count);
size_t write(fd_t fd, const void *buf, size_t count);
int resmgr_attach(const char *pathname);

#if 0
union fs_message{
  struct {
    u32_t cmd;
  }data;

  struct {
    u32_t cmd;
    offs_t offset;
  }data1;

  struct {
    u32_t cmd;
    offs_t offset;
  }data2;

  struct {
    u32_t cmd;
    offs_t offset;
    char buf[FS_CMD_LEN];
  }data3;
} __attribute__ ((packed));
#endif

#endif
