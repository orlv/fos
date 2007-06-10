/*
  include/fs.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef __FS_H
#define __FS_H

#include <types.h>

#define MAX_PATH_LEN 1024
#define MAX_NAME_LEN 256

#define FS_CMD_ACCESS 0
#define FS_CMD_READ   1
#define FS_CMD_WRITE  2
#define FS_CMD_LIST   3

#define NAMER_CMD_ACCESS  0
#define NAMER_CMD_ADD     4
#define NAMER_CMD_REM     5
#define NAMER_CMD_RESOLVE 6

#define PID_NAMER 0

struct fs_message {
  u32_t cmd;
  char buf[MAX_PATH_LEN];
} __attribute__ ((packed));

#endif
