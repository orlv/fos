#include <fos/message.h>
#include <fos/fs.h>
#include "privatetypes.h"

extern fd_t __gui;
void SetFocus(int handle) {
  struct message msg;

  msg.flags = 0;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = __gui->thread;
  msg.arg[0] = WIN_CMD_SETFOCUS;
  msg.arg[1] = handle;
  send(&msg);
}
