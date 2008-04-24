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

  size_t len = strlen(oldpath) + 1 + strlen(newpath) + 1;
  if(len > MAX_PATH_LEN) {
    errno = ENAMETOOLONG;
    return -1;
  }
  struct message msg;
  msg.arg[0] = FS_CMD_RENAME;
  char *pack = malloc(len);
  strcpy(pack, oldpath);
  strcpy(pack + strlen(oldpath) + 1, newpath);
  msg.send_buf = pack;
  msg.send_size = len;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;
  u32_t result = send((struct message *)&msg);
  free(pack);
  if(result == RES_SUCCESS && msg.arg[2] == NO_ERR)
    return 0;
  return -1;
}

