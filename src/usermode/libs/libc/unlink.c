#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fos/message.h>

int unlink(const char *pathname) {
  if(pathname[0] != '/') {
    char *pwd = getenv("PWD");
    if(!pwd) return -1;
    if(pwd[0] != '/') return -1;
    char *buf = malloc(strlen(pathname) + strlen(pwd) + 1);
    strcpy(buf, pwd); strcat(buf, pathname);
    int ret = unlink(buf);
    free(buf);
    return ret;
  }
  struct message msg;
  msg.arg[0] = FS_CMD_UNLINK;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return -1;
  msg.send_buf = pathname;
  msg.send_size = strlen(pathname)+1;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;
  u32_t result = send((struct message *)&msg);
  if(result == RES_SUCCESS && msg.arg[2] == NO_ERR)
    return 0;
  return -1;
}
