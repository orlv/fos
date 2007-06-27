/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <vsprintf.h>
#include "vga.h"
#include "tty.h"
#include <fs.h>

extern tid_t procman;
extern tid_t namer;

asmlinkage int main()
{
  while(!(procman = resolve("/sys/procman")));

  u32_t res;
  fs_message *m = new fs_message;
  message *msg = new message;

  resmgr_attach("/dev/tty");
  
  VGA *vga = new VGA;
  vga->init();
  TTY *tty = new TTY(80, 25);
  tty->stdout = vga;

  char *dmesgbuf = new char[2000];
  size_t len = dmesg(dmesgbuf, 2000);

  tty->write(0, dmesgbuf, len);

  while (1) {
    msg->tid = 0;
    msg->recv_size = sizeof(fs_message);
    msg->recv_buf = m;
    receive(msg);

    switch(m->data3.cmd){
    case FS_CMD_ACCESS:
      res = RES_SUCCESS;
      break;

    case FS_CMD_WRITE:
      tty->write(0, m->data3.buf, msg->recv_size - 9);
      res = RES_SUCCESS;
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
