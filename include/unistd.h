/*
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _UNISTD_H
#define _UNISTD_H  1

#include <types.h>
#include <fos/page.h>
#include <sys/stat.h>

#define getpagesize()     (PAGE_SIZE)

userlinkage int close(int fildes);
userlinkage ssize_t read(int fildes, void *buf, size_t bytes);
userlinkage ssize_t write(int fildes, const void *buf, size_t bytes);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

userlinkage off_t lseek(int fildes, off_t offset, int whence);

int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;
extern char **environ;

userlinkage char *getcwd(char *buf, size_t size);
userlinkage int chdir(const char *path);

userlinkage int unlink(const char *pathname);
userlinkage int rmdir(const char *pathname);
userlinkage int rename(const char *oldpath, const char *newpath);

userlinkage int access(const char *pathname, int mode);

#define F_OK	(1 << 0)
#define R_OK	(1 << 1)
#define W_OK	(1 << 2)
#define X_OK	(1 << 3)
#endif
