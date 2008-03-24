/*
  Copyright (C) 2007 Oleg Fedorov
  Serial I/O (C) 2007 Gridassov Sergey
*/

#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <stdlib.h>
#include "tty.h"
#include <fos/nsi.h>

#define RECV_BUF_SIZE 2048

static char *buffer;

void access(struct message *msg)
{
  msg->arg[0] = 1;
  msg->arg[1] = RECV_BUF_SIZE;
  msg->arg[2] = NO_ERR;
  msg->send_size = 0;
}

void write(struct message *msg)
{
  msg->arg[0] = tty_write(0, buffer, msg->recv_size);
  msg->arg[2] = NO_ERR;
  msg->send_size = 0;
}

asmlinkage int main()
{
  tty_init();
  buffer = (char *) malloc(RECV_BUF_SIZE);
  nsi_t *interface = new nsi_t("/dev/tty");

  /* стандартные параметры */
  interface->std.recv_buf = buffer;
  interface->std.recv_size = RECV_BUF_SIZE;

  /* объявим интерфейсы */
  interface->add(FS_CMD_ACCESS, &access);
  interface->add(FS_CMD_WRITE, &write);

  /* обрабатываем поступающие сообщения */
  while (1) {
    interface->wait_message();
  };

  return 0;
}
