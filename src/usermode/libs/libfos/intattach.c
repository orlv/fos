/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

/*
  при возникновении указанного прерывания данному потоку
  будет приходить сообщение
*/
res_t interrupt_attach(u8_t n)
{
  struct message msg;
  msg.arg[0] = PROCMAN_CMD_INTERRUPT_ATTACH;
  msg.arg[1] = n;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.arg[0];
  else
    return 0;
}
