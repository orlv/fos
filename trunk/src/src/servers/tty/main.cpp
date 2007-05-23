/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <vsprintf.h>
#include "vga.h"
#include "tty.h"
#include <fs.h>

asmlinkage void _start()
{
  u32_t res;
  struct fs_message *m = new fs_message;
  struct message *msg = new(struct message);
  
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(struct fs_message);
  msg->send_buf = m;
  strcpy(m->name, "/dev/tty");
  m->cmd = NAMER_CMD_ADD;
  msg->pid = PID_NAMER;
  send(msg);

  char *buf = new char[256];
  VGA *vga = new VGA;

  vga->init();
  TTY *tty = new TTY(80, 25);
  tty->stdout = vga;

  tty->outs("Console Activated \n");

  //tty->outs("tty: msg to namer send \n");

  
  exec("fs");
  exec("app1");
  
  while (1) {
    msg->pid = 0;
    msg->recv_size = 256;
    msg->recv_buf = buf;
    receive(msg);
    tty->outs(buf);
    msg->send_buf = buf;
    msg->send_size = 2;
    strcpy(buf, "OK");
    reply(msg);
  }
}
