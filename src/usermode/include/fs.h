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

#define NAMER_CMD_ACCESS  0
#define NAMER_CMD_ADD     4
#define NAMER_CMD_REM     5
#define NAMER_CMD_RESOLVE 6

#define PID_NAMER 0

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

struct fd {
  tid_t thread;
  offs_t offset;
};

typedef struct fd* fd_t;

asmlinkage fd_t open(const char *pathname, int flags);
asmlinkage int close(fd_t fd);
asmlinkage size_t read(fd_t fd, void *buf, size_t count);
asmlinkage size_t write(fd_t fd, void *buf, size_t count);

#endif
