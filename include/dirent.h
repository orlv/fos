/*
  include/dirent.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _DIRENT_H
#define _DIRENT_H
#include <types.h>
#define NAME_MAX 256
struct dirent {
    long d_ino;                 /* inode number */
    off_t d_off;                /* offset to this dirent */
    unsigned short d_reclen;    /* length of this d_name */
    char d_name [NAME_MAX + 1];   /* filename (null-terminated) */
};

typedef void DIR;


DIR *opendir(const char *name);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);
void seekdir(DIR *dir, off_t offset);
void rewinddir(DIR *dir);
#endif
