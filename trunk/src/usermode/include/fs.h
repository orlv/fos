/*
  include/fs.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef __FS_H
#define __FS_H

#include <types.h>

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

struct fd {
  tid_t  thread;
  offs_t offset;
  u32_t  id;
};

typedef struct fd* fd_t;

asmlinkage int open(const char *pathname, int flags);
asmlinkage int close(int fildes);
asmlinkage ssize_t read(int fildes, void *buf, size_t bytes);
asmlinkage ssize_t write(int fildes, const void *buf, size_t bytes);

#endif
