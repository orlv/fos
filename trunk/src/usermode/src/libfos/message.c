/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/syscall.h>
#include <fos/message.h>

res_t receive(struct message *msg)
{
  return sys_call(_FOS_RECEIVE, (u32_t) msg);
}

res_t send(struct message *msg)
{
  return sys_call(_FOS_SEND, (u32_t) msg);
}

res_t reply(struct message *msg)
{
  return sys_call(_FOS_REPLY, (u32_t) msg);
}

res_t forward(struct message *msg, tid_t to)
{
  return sys_call2(_FOS_FORWARD, (u32_t) msg, to);
}
