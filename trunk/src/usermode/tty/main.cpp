/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <vsprintf.h>
#include "vga.h"
#include "tty.h"
#include <fs.h>

asmlinkage int main()
{
  message *msg = new message;
  char *buffer = new char[2048];

  VGA *vga = new VGA;
  vga->init();
  TTY *tty = new TTY(80, 25);
  tty->stdout = vga;

  size_t len = dmesg(buffer, 2048);

  tty->write(0, buffer, len);

  if(resmgr_attach("/dev/tty") != RES_SUCCESS)
    return -1;
  
  while (1) {
    msg->tid = _MSG_SENDER_ANY;
    msg->recv_size = 2048;
    msg->recv_buf = buffer;
    receive(msg);
    switch(msg->a0){
    case FS_CMD_ACCESS:
      msg->a0 = 1;
      break;

    case FS_CMD_WRITE:
      msg->a0 = tty->write(0, buffer, msg->recv_size);
      break;

    default:
      msg->a0 = 0;
    }
    
    msg->send_size = 0;
    reply(msg);
  }
  return 0;
}
