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
#define FS_CMD_STAT   4
#define FS_CMD_FSTAT  5

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

#include <time.h>

typedef sid_t dev_t;
typedef size_t nlink_t;
typedef size_t blksize_t;
typedef size_t blkcnt_t;

struct stat {
  dev_t     st_dev;     /* ID of device containing file */
  ino_t     st_ino;     /* inode number */
  mode_t    st_mode;    /* protection */
  nlink_t   st_nlink;   /* number of hard links */
  uid_t     st_uid;     /* user ID of owner */
  gid_t     st_gid;     /* group ID of owner */
  dev_t     st_rdev;    /* device ID (if special file) */
  off_t     st_size;    /* total size, in bytes */
  blksize_t st_blksize; /* blocksize for filesystem I/O */
  blkcnt_t  st_blocks;  /* number of blocks allocated */
  time_t    st_atime;   /* time of last access */
  time_t    st_mtime;   /* time of last modification */
  time_t    st_ctime;   /* time of last status change */
};

int stat(const char *path, struct stat *buf);
int fstat(int fildes, struct stat *buf);

#endif
