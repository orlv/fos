/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include "keyboard.h"
#include <fs.h>

#define KB_BUF_SIZE 64
#define PORT_KBD_A      0x60

#define KEYBOARD_IRQ_NUM 1

asmlinkage int main()
{
  printf("Usermode keyboard driver\n");
  res_t res = interrupt_attach(KEYBOARD_IRQ_NUM);
  printf("keyboard: res=%d\n", res);

  unmask_interrupt(KEYBOARD_IRQ_NUM);
  
  struct message msg;
  u8_t num;
  char scancode;
  while(1){
    msg.recv_size = sizeof(num);
    msg.recv_buf = &num;
    receive(&msg);
    //printf("keyboard: %d\n", num);
    scancode = inportb(PORT_KBD_A);
    unmask_interrupt(num);
    printf("scancode=0x%X\n", scancode);
  }

#if 0
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
#endif
  return 0;
}
