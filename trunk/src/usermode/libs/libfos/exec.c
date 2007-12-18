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
#include <unistd.h>

tid_t exece(const char * filename, const char * args, const char **envp)
{

  char *send_data = (char *) filename;
  size_t len = strlen(filename) + 1;
  if(len > MAX_PATH_LEN)
    return 0;

  size_t send_size = len;
  size_t args_len = 0;
  size_t envp_len = 0;
  
  if(args) {
    args_len = strlen(args) + 1;
    if(args_len + len > ARG_MAX)
      return -1;
  }

  if(envp) {
#warning варнинг строкой ниже исправлю, когда функции среды будут. Тут надо правильный замер длины строки.
    envp_len = strlen(envp) + 1; // TODO: здесь мерять надо иначе! будет среда, будет и это.
    if(envp_len > ENVP_MAX)
      return -1;
  }
  send_size += args_len + envp_len;
  send_size += len;
  send_data = (char *) malloc(send_size);
  memcpy(send_data, filename, len);
  memcpy(&send_data[len], filename, len);
  args_len += len;
  if(args_len - len) memcpy(&send_data[len + len], args, args_len);
  if(envp_len) memcpy(&send_data[len + args_len + len], envp, envp_len);
  
  struct message msg;
  msg.arg[0] = PROCMAN_CMD_EXEC;
  msg.send_buf = send_data;
  msg.send_size = send_size;
  msg.arg[1] = args_len;
  msg.arg[2] = envp_len;
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
  return exece(filename, args, (const char **)environ);
}
