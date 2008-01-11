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
  char *plain_env = NULL;
  size_t len = strlen(filename) + 1;
  if(len > MAX_PATH_LEN)
    return 0;

  size_t send_size = len;
  size_t args_len = 0;
  size_t envp_len = 0;
  
  if(args) {    args_len = strlen(args) + 1;
    if(args_len + len > ARG_MAX)
      return -1;
  }

  if(envp) {
    for(int i = 0; envp[i]; i++) {
      envp_len += strlen(envp[i]) + 1;
    }
    envp_len++;
    if(envp_len > ENVP_MAX)
      return -1;
    plain_env = malloc(envp_len);
    if(!plain_env)
      return -1;
    char *envptr = plain_env;
    for(int i = 0; envp[i]; i++) {
      int len = strlen(envp[i]) + 1;
      memcpy(envptr, envp[i], len);
      envptr += len;
    }
  }
   args_len += len;
  send_size += args_len + envp_len;
  send_data = (char *) malloc(send_size);
  memcpy(send_data, filename, len);
  memcpy(&send_data[len], filename, len);

  if(args_len - len) {
	send_data[len + len - 1] = ' ';
	memcpy(&send_data[len + len], args, args_len);
  }
  if(envp_len) 
	memcpy(&send_data[len + args_len], plain_env, envp_len);
  
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

  free(send_data);
  
  if(res == RES_SUCCESS)
    return (tid_t) msg.arg[0];
  else
    return 0;
}

tid_t exec(const char * filename, const char * args)
{
  return exece(filename, args, (const char **)environ);
//	return exece(filename, args, NULL);
}
