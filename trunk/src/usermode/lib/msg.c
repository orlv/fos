/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>

asmlinkage res_t receive(struct message *msg)
{
  return sys_call(_FOS_RECEIVE, (u32_t) msg);
}

asmlinkage res_t send(struct message *msg)
{
  return sys_call(_FOS_SEND, (u32_t) msg);
}

asmlinkage res_t reply(struct message *msg)
{
  return sys_call(_FOS_REPLY, (u32_t) msg);
}
