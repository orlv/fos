/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>

asmlinkage int main()
{
  printf("{Hello app1}\n");

#if 1
  struct message *msg = new struct message;
  u32_t res;
  struct fs_message *m = new fs_message;
  tid_t keyboard;
  while(!(keyboard = resolve("/dev/keyboard")));

  printf("app1: keyboard tid=0x%X\n", keyboard);
  
  while(1){
    msg->recv_size = sizeof(res);
    msg->recv_buf = &res;
    msg->send_size = sizeof(struct fs_message);
    m->cmd = FS_CMD_READ;
    msg->send_buf = m;
    msg->tid = keyboard;
    send(msg);
    printf("%c", res);
  }
#endif
  return 0;
}
