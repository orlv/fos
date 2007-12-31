/*
  Copyright (C) 2007 Serge Gridassov
 */

#include <fos/message.h>
#include <unistd.h>
#include "privatetypes.h"

extern fd_t __gui;

void SetVisible(int handle, int visible)
{
  struct win_hndl *wh = (struct win_hndl *)handle;
  struct message msg;

  msg.flags = 0;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = __gui->thread;
  msg.arg[0] = WIN_CMD_SETVISIBLE;
  msg.arg[1] = wh->handle;
  msg.arg[2] = visible;
  send(&msg);
}
