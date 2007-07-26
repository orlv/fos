/*
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H    1

#include <types.h>
#include <fos/page.h>
#include <fos/fs.h>
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

asmlinkage int stat(const char *path, struct stat *buf);
asmlinkage int fstat(int fildes, struct stat *buf);

#endif
