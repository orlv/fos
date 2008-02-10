/*
  include/fos/message.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_MESSAGE_H
#define _FOS_MESSAGE_H

#include <types.h>
#include <sched.h>

//#define _MSG_SENDER_ANY    0
//#define _MSG_SENDER_SIGNAL 1

#define SIGNAL_ALARM   0
#define SIGNAL_IRQ     1

#define SYSTID_PROCMAN 2
#define SYSTID_MM      3
#define SYSTID_NAMER   6

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

#ifdef iKERNEL

#include <c++/atomic.h>
#include <c++/list.h>

class kmessage {
 public:
  void * buffer;
  size_t size;
  size_t reply_size;
  class Thread * volatile thread;
  u32_t flags;
  arg_t  arg[MSG_ARGS_CNT];
  void check_size(message *msg);
};

class msg_list {
 public:
  List<kmessage *> list;
  atomic_t count;
  List<kmessage *> *get(Thread *sender, u32_t flags);
};

class Messenger {
 private:
  class Thread *thread;
 public:
  Messenger(class Thread *thread) {
    this->thread = thread;
  }

  msg_list unread;
  msg_list read;

  /* переместить сообщение в пространство процесса */
  kmessage *import(message *msg, Thread *sender);
  void move_to_userspace(kmessage *kmsg, message *msg);

  res_t put_message(kmessage *message);
  /* находит сообщение в списке непрочтённых */
  kmessage *get(Thread *sender, u32_t flags);
};

#endif

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
