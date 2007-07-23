/*
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _UNISTD_H
#define _UNISTD_H  1

#include <types.h>
#include <fos/page.h>
#include <sys/stat.h>

#define getpagesize()     (PAGE_SIZE)

asmlinkage int close(int fildes);
asmlinkage ssize_t read(int fildes, void *buf, size_t bytes);
asmlinkage ssize_t write(int fildes, const void *buf, size_t bytes);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

asmlinkage off_t lseek(int fildes, off_t offset, int whence);

#endif
