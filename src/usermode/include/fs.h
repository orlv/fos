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

#define NO_ERR                0
#define ERR_EOF               1
#define ERR_LOCALBUF_OVERFULL 2
#define ERR_NO_SUCH_FILE      3
#define ERR_UNKNOWN_CMD       4
#define ERR_ACCESS_DENIED     5

typedef u32_t ino_t;

struct fd {
  tid_t  thread;
  off_t offset;
  ino_t  inode;
  size_t  buf_size;
};

typedef struct fd* fd_t;

asmlinkage int open(const char *pathname, int flags);
asmlinkage int close(int fildes);
asmlinkage ssize_t read(int fildes, void *buf, size_t bytes);
asmlinkage ssize_t write(int fildes, const void *buf, size_t bytes);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

asmlinkage off_t lseek(int fildes, off_t offset, int whence);

#if 0
struct dirent {
  ino_t          d_ino;       /* inode number */
  off_t          d_off;       /* offset to the next dirent */
  unsigned short d_reclen;    /* length of this record */
  unsigned char  d_type;      /* type of file */
  char           d_name[MAX_NAME_LEN]; /* filename */
} __attribute__ ((packed));

asmlinkage struct dirent *readdir(DIR *dir);
#endif

#endif
