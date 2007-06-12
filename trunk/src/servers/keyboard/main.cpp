/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include "keyboard.h"
#include <fs.h>

volatile bool ready = 0;
Keyboard *kb;

void thread_handler()
{
  if(interrupt_attach(KEYBOARD_IRQ_NUM) == RES_SUCCESS){
    printf("keyboard: interrupt attached\n");
  } else {
    printf("keyboard: can't attache interrupt!\n");
    exit();
  }

  struct message msg;
  u8_t num;
  kb = new Keyboard;
  ready = 1;  

  while(1){
    msg.recv_size = sizeof(num);
    msg.recv_buf = &num;
    receive(&msg);
    if(num == KEYBOARD_IRQ_NUM){
      kb->handler();
    } else {
      printf("Keyboard handler: unknown message received!\n");
    }
  }
}

asmlinkage int main()
{
  printf("Usermode keyboard driver\n");
  thread_create((off_t) &thread_handler);

  while(!ready);

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

  while (1) {
    msg->tid = 0;
    msg->recv_size = sizeof(struct fs_message);
    msg->recv_buf = m;
    receive(msg);
    
    switch(m->cmd){
    case FS_CMD_ACCESS:
      res = RES_SUCCESS;
      break;

    case FS_CMD_WRITE:
      kb->put(m->buf[0]);
      res = RES_SUCCESS;
      break;

    case FS_CMD_READ:
      res = kb->get();
      break;

    default:
      res = RES_FAULT;
    }
    
    msg->send_buf = &res;
    msg->send_size = sizeof(res);
    reply(msg);
  }

  return 0;
}
