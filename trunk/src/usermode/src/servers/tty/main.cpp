/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include "vga.h"
#include "tty.h"


#define RECV_BUF_SIZE 2048

asmlinkage int main()
{
  message msg;
  char *buffer = new char[RECV_BUF_SIZE];

  VGA *vga = new VGA;
  vga->init();
  TTY *tty = new TTY(80, 25);
  tty->stdout = vga;

  size_t len = dmesg(buffer, RECV_BUF_SIZE);

  tty->write(0, buffer, len);

  if(resmgr_attach("/dev/tty") != RES_SUCCESS)
    return -1;
  
  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_size = RECV_BUF_SIZE;
    msg.recv_buf = buffer;
    receive(&msg);
    switch(msg.a0){
    case FS_CMD_ACCESS:
      msg.a0 = 1;
      msg.a1 = RECV_BUF_SIZE;
      msg.a2 = NO_ERR;
      break;

    case FS_CMD_WRITE:
      msg.a0 = tty->write(0 /*msg.a2*/, buffer, msg.recv_size);
      msg.a2 = NO_ERR;
      break;

    default:
      msg.a0 = 0;
      msg.a2 = ERR_UNKNOWN_CMD;
    }
    
    msg.send_size = 0;
    reply(&msg);
  }
  return 0;
}
