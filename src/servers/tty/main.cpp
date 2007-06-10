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
  while(!(namer = resolve("/sys/namer")));
  while(!(procman = resolve("/sys/procman")));

  u32_t res;
  struct fs_message *m = new fs_message;
  struct message *msg = new(struct message);
  
  msg->recv_size = sizeof(res);
  msg->recv_buf = &res;
  msg->send_size = sizeof(struct fs_message);
  msg->send_buf = m;
  strcpy(m->buf, "/dev/tty");
  m->cmd = NAMER_CMD_ADD;
  msg->tid = namer;
  send(msg);

  VGA *vga = new VGA;

  vga->init();
  TTY *tty = new TTY(80, 25);
  tty->stdout = vga;

  tty->outs("Console Activated \n");

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
      tty->outs(m->buf);
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
