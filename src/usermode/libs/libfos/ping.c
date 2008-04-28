/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <string.h>
#include <errno.h>

int ping(const char *target) {
  struct message msg;
  msg.arg[0] = FS_CMD_PING;
  size_t len = strlen(target) + 1;
  if(len > MAX_PATH_LEN) {
    errno = ENAMETOOLONG;
    return -1;
  }
  msg.send_buf = target;
  msg.send_size = len;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;
  u32_t result = send((struct message *)&msg);

  if(result == RES_SUCCESS && msg.arg[2] == NO_ERR)
    return 0;
  else if(result != RES_SUCCESS) {
    errno = ENOENT;
  }
  return -1;
}
