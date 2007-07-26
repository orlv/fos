/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyboard.h"

volatile bool ready = 0;
Keyboard *kb;

void thread_handler()
{
  if(interrupt_attach(KBD_IRQ_NUM) == RES_SUCCESS)
    printf("keyboard: interrupt attached\n");
  else {
    printf("keyboard: can't attache interrupt!\n");
    exit(1);
  }

  struct message msg;
  kb = new Keyboard;
  ready = 1;
  
  while(1){
    msg.recv_size = 0;
    msg.tid = _MSG_SENDER_SIGNAL;
    receive(&msg);
    if(msg.a0 == KBD_IRQ_NUM)
      kb->handler();
    else
      printf("Keyboard handler: unknown signal received!\n");
  }
}

asmlinkage int main()
{
  printf("Usermode keyboard driver\n");
  thread_create((off_t) &thread_handler);

  while(!ready);

  struct message msg;
  char *buffer = new char[KB_CHARS_BUFF_SIZE];

  resmgr_attach("/dev/keyboard");

  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_buf  = buffer;
    msg.recv_size = KB_CHARS_BUFF_SIZE;
    receive(&msg);

    switch(msg.a0){
    case FS_CMD_ACCESS:
      msg.a0 = 1;
      msg.a1 = KB_CHARS_BUFF_SIZE;
      msg.a2 = NO_ERR;
      msg.send_size = 0;
      break;

    case FS_CMD_WRITE:
      msg.a0 = kb->write(0, buffer, msg.recv_size);
      msg.send_size = 0;
      if(msg.a0 < msg.recv_size)
	msg.a2 = ERR_EOF;
      else
	msg.a2 = NO_ERR;
      break;

    case FS_CMD_READ:
      msg.a0 = kb->read(0, buffer, msg.send_size);
      if(msg.a0 < msg.send_size) {
	msg.send_size = msg.a0;
	msg.a2 = ERR_EOF;
      } else
	msg.a2 = NO_ERR;

      msg.send_buf = buffer;
      break;

    default:
      msg.a0 = 0;
      msg.a2 = ERR_UNKNOWN_CMD;
      msg.send_size = 0;
    }
    
    reply(&msg);
  }

  return 0;
}
