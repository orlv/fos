#include <fos.h>
#include <string.h>
#include <vsprintf.h>
#include "vga.h"
#include "tty.h"

asmlinkage void _start()
{
  struct msg *msg = new(struct msg);
  char *buf = new char[256];
  VGA *vga = new VGA;
  vga->init();

  TTY *tty = new TTY(80, 25);
  tty->stdout = vga;

  tty->outs("Console Activated \n");

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
