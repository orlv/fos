/*
  include/fos/fs.h
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#ifndef _FOS_FS_H
#define _FOS_FS_H    1

#include <types.h>
#include <mutex.h>

#define MAX_PATH_LEN 1024
#define MAX_NAME_LEN 256
#define FS_CMD_LEN   1024

#define FS_CMD_ACCESS 0
#define FS_CMD_READ   1
#define FS_CMD_WRITE  2
#define FS_CMD_LIST   3
#define FS_CMD_STAT   4
#define FS_CMD_FSTAT  5
#define FS_CMD_CLOSE  6

#define FS_CMD_DIROPEN	7
#define FS_CMD_DIRCLOSE	8
#define FS_CMD_DIRREAD	9

#define FS_CMD_UNLINK 	10

#define BASE_CMD_N 64

#define NO_ERR                0
#define ERR_EOF               1
#define ERR_LOCALBUF_OVERFULL 2
#define ERR_NO_SUCH_FILE      3
#define ERR_UNKNOWN_CMD       4
#define ERR_ACCESS_DENIED     5
#define ERR_TIMEOUT           6

typedef u32_t ino_t;

struct fd {
  tid_t  thread;
  off_t offset;
  ino_t  inode;
  size_t  buf_size;
  size_t file_size;
  int flags;
  const char *fullname;
  //  char *name;
};

typedef struct fd* fd_t;

struct xfer_databuf {
  size_t data_size;
  off_t name_offs;
};

#include <dirent.h>

struct dirfd {
  tid_t	thread;
  ino_t	inode;
  off_t	offset;
  size_t	inodes;
  const char *fullname;
  struct dirent ent;
  //  char *name;
};

#define xfer_databuf_of(ptr, size) ({\
      (struct xfer_databuf *)((char *) ptr + size - sizeof(struct xfer_databuf));})

typedef struct {
	int open_mode;
	int allow_read;
	int allow_write;
	mutex_t using_mutex;
	char *buf;
	int buf_size;
	int buf_ptr;
	int handle;
} __fopen_fd;

#endif
