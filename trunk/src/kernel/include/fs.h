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

#define NO_ERR                0
#define ERR_EOF               1
#define ERR_LOCALBUF_OVERFULL 2
#define ERR_NO_SUCH_FILE      3
#define ERR_UNKNOWN_CMD       4
#define ERR_ACCESS_DENIED     5

typedef u32_t ino_t;

struct fd {
  tid_t  thread;
  offs_t offset;
  ino_t  inode;
  size_t  buf_size;
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

int close(int fildes);
int open(const char *pathname, int flags);
ssize_t read(int fildes, void *buf, size_t nbyte);
ssize_t write(int fildes, const void *buf, size_t nbyte);
int resmgr_attach(const char *pathname);

#endif