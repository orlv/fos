/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>

asmlinkage void receive(struct message *msg)
{
  sys_call(_FOS_RECEIVE, (u32_t) msg);
}

asmlinkage void send(struct message *msg)
{
  sys_call(_FOS_SEND, (u32_t) msg);
}

asmlinkage void reply(struct message *msg)
{
  sys_call(_FOS_REPLY, (u32_t) msg);
}
