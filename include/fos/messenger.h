/*
  include/fos/messenger.h
  Copyright (C) 2007-2008 Oleg Fedorov
 */

#ifndef _FOS_MESSENGER_H
#define _FOS_MESSENGER_H

#include <types.h>
#include <c++/atomic.h>
#include <c++/list.h>
#include <fos/message.h>

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
