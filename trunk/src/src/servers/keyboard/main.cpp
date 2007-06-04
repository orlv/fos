/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include "keyboard.h"
#include <fs.h>

#define KB_BUF_SIZE 64

asmlinkage int main()
{
  u32_t res;
  struct fs_message *m = new fs_message;
  struct message *msg = new message;
  
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(struct fs_message);
  msg->send_buf = m;
  strcpy(m->buf, "/dev/keyboard");
  m->cmd = NAMER_CMD_ADD;
  msg->tid = PID_NAMER;
  send(msg);

  printf("[Keyboard]\n");
  Keyboard *kb = new Keyboard;

  while (1) {
    msg->tid = 0;
    msg->recv_size = sizeof(struct fs_message);
    msg->recv_buf = m;
    receive(msg);


    //printf("keyboard: rcvd msg!\n");
    switch(m->cmd){
    case FS_CMD_ACCESS:
      res = RES_SUCCESS;
      break;

    case FS_CMD_WRITE:
      kb->put(m->buf[0]);
      printf("[%X]",m->buf[0]);
      res = RES_SUCCESS;
      break;

      /*    case FS_CMD_READ:
      res = kb->get();
      break;*/

    default:
      res = RES_FAULT;
    }
    
    msg->send_buf = &res;
    msg->send_size = sizeof(res);
    reply(msg);
  }

  return 0;
}
