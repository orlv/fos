/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <fos/fs.h>
#include <fos/message.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int rename(const char *oldpath, const char *newpath) {
  if(oldpath[0] != '/') {
    char *pwd = getenv("PWD");
    if(!pwd) {
      errno = ENOMEM;
      return -1;
    }
    if(pwd[0] != '/') {
      errno = ENOENT;
      return -1;
    }
    char *buf = malloc(strlen(oldpath) + strlen(pwd) + 1);
    strcpy(buf, pwd); strcat(buf, oldpath);
    int ret = rename(buf, newpath);
    free(buf);
    return ret;
  }
  if(newpath[0] != '/') {
    char *pwd = getenv("PWD");
    if(!pwd) {
      errno = ENOMEM;
      return -1;
    }
    if(pwd[0] != '/') {
      errno = ENOENT;
      return -1;
    }
    char *buf = malloc(strlen(newpath) + strlen(pwd) + 1);
    strcpy(buf, pwd); strcat(buf, newpath);
    int ret = rename(oldpath, newpath);
    free(buf);
    return ret;
  }
  printf("Renaming %s to %s\n");

  exit(1);
  return 666;
}

