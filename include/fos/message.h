/*
  include/fos/message.h
  Copyright (C) 2007-2008 Oleg Fedorov
 */

#ifndef _FOS_MESSAGE_H
#define _FOS_MESSAGE_H

#include <types.h>
#include <sched.h>

//#define _MSG_SENDER_ANY    0
//#define _MSG_SENDER_SIGNAL 1

#define SIGNAL_ALARM   0
#define SIGNAL_IRQ     1

#define SYSTID_PROCMAN 1
#define SYSTID_MM      2
#define SYSTID_NAMER   5

#define MAX_MSG_COUNT 32

#define MSG_ASYNC     1
#define MSG_MEM_SEND  2 /* отделить страницу от адресного пространства, и присоединить к получателю */
#define MSG_MEM_SHARE 4 /* клиент: присоединить страницу в адр. пр-во получателя
			   или сервер: разрешение на присоединение разделяемой страницы */
#define MSG_MEM_TAKE  2 /* готовность получить страницу памяти */

#define MSG_ARGS_CNT  4
typedef u32_t arg_t;

struct message {
  const void * send_buf;
  size_t send_size;

  void * recv_buf;
  size_t recv_size;

  pid_t  pid;
  tid_t  tid;
  
  u32_t  flags;
  arg_t  arg[MSG_ARGS_CNT];
};

asmlinkage res_t send(struct message *msg);
asmlinkage res_t receive(struct message *msg);
asmlinkage res_t reply(struct message *msg);
asmlinkage res_t forward(struct message *msg, tid_t to);

static inline res_t do_send(struct message *msg)
{
  res_t result;
  while(1) {
    result = send(msg);
    if(result == RES_FAULT2) { /* очередь получателя переполнена, обратимся чуть позже */
      sched_yield();
      continue;
    }
    return result;
  }
}

#endif
