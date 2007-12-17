/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/syscall.h>
#include <fos/fs.h>
#include <fos/procman.h>
#include <fos/limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

tid_t exece(const char * filename, const char * args, const char * envp)
{
  char *send_data = (char *) filename;
  size_t len = strlen(filename);
  if(len+1 > MAX_PATH_LEN)
    return 0;

  size_t send_size = len + 1;
  size_t args_len = 0;
  size_t envp_len = 0;
  
  if(args) {
    args_len = strlen(args);
    if(args_len > ARG_MAX)
      return -1;
  }

  if(envp) {
    envp_len = strlen(envp);
    if(envp_len > ENVP_MAX)
      return -1;
  }
    
  if(args_len + envp_len) {
    send_size += args_len + 1 + envp_len + 1;
    send_data = (char *) malloc(send_size);
    memcpy(send_data, filename, len+1);
    if(args_len) memcpy(&send_data[len+1], args, args_len);
    if(envp_len) memcpy(&send_data[len+1+args_len+1], envp, envp_len);
  }
  
  struct message msg;
  msg.arg[0] = PROCMAN_CMD_EXEC;
  msg.send_buf = send_data;
  msg.send_size = send_size;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_PROCMAN;
  res_t res = send(&msg);

  if(args_len) free(send_data);
  
  if(res == RES_SUCCESS)
    return (tid_t) msg.arg[0];
  else
    return 0;
}

tid_t exec(const char * filename, const char * args)
{
  return exece(filename, args, 0);
}
