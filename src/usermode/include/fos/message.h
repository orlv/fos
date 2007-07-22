/*
  include/fos/message.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_MESSAGE_H
#define _FOS_MESSAGE_H

#include <types.h>

#define _MSG_SENDER_ANY    0
#define _MSG_SENDER_SIGNAL 1

#define SYSTID_NAMER   1
#define SYSTID_PROCMAN 2

struct message {
  const void * send_buf;
  size_t send_size;

  void * recv_buf;
  size_t recv_size;

  tid_t  tid;

  u32_t  a0;
  u32_t  a1;
  u32_t  a2;
  u32_t  a3;
} __attribute__ ((packed));

asmlinkage res_t send(struct message *msg);
asmlinkage res_t receive(struct message *msg);
asmlinkage res_t reply(struct message *msg);
asmlinkage res_t forward(struct message *msg, tid_t to);

#endif
