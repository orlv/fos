#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyboard.h"
#include "mouse.h"

void thread_handler()
{
  if(interrupt_attach(1) == RES_SUCCESS)
    printf("keyboard: interrupt attached\n");
  else {
    printf("keyboard: can't attach interrupt!\n");
    exit(1);
  }
  unmask_interrupt(1);
  if(interrupt_attach(12) == RES_SUCCESS)
    printf("mouse: interrupt attached\n");
  else {
    printf("mouse: can't attach interrupt!\n");
    exit(1);
  }
  unmask_interrupt(12);
  struct message msg;
  while(1){
    msg.recv_size = 0;
    msg.tid = _MSG_SENDER_SIGNAL;
    receive(&msg);
    if(msg.a0 == 1) {
      keyboard_ps2_interrupt();
    } else if(msg.a0 == 12) {
      mouse_ps2_interrupt();
    }
    else
      printf("i8042: unknown signal received!\n");
  }
}

int main(int argc, char *argv[])
{
  printf("i8042: starting up\n");
  printf("i8042: keyboard init\n");
  keyboard_ps2_init();
  
  printf("i8042: mouse init\n");
  mouse_ps2_init();
  printf("i8042: interface init\n");
  
  struct message msg;
  char *buffer = malloc(KB_CHARS_BUFF_SIZE);
  
  printf("i8042: daemonized\n");
  thread_create((off_t) &MouseHandlerThread);
  thread_create((off_t) &thread_handler);
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
      msg.a0 = kb_write(0, buffer, msg.recv_size);
      msg.send_size = 0;
      if(msg.a0 < msg.recv_size)
	msg.a2 = ERR_EOF;
      else
	msg.a2 = NO_ERR;
      break;
      
    case FS_CMD_READ:
      msg.a0 = kb_read(0, buffer, msg.send_size);
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
}
