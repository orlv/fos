/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>

asmlinkage void thread1()
{
  printf("Hello thread1\n");
  while(1){
    printf(".");
  }
}

asmlinkage int main()
{
  printf("{Hello app1}\n");
  thread_create((off_t) &thread1);
  while(1) printf("*");
#if 0
  struct message *msg = new struct message;
  u32_t res;
  struct fs_message *m = new fs_message;
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(struct fs_message);
  msg->send_buf = m;
  strcpy(m->buf, "/dev/keyboard");
  m->cmd = FS_CMD_ACCESS;
  msg->tid = PID_NAMER;
  send(msg);

  printf("app1: msg send & reply=%d from %d.\n", res, msg->tid);
  while(1);// printf(".");
  
  while(1){
    msg->recv_size = sizeof(res);
    msg->recv_buf = &res;
    msg->send_size = sizeof(struct fs_message);
    m->cmd = FS_CMD_READ;
    msg->send_buf = m;
    send(msg);
    printf("[%X]", res);
  }
#endif
  return 0;
}
