/*
  kernel/include/process.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _PROCESS_H
#define _PROCESS_H

#include <types.h>
#include <tss.h>
#include <list.h>
#include <gdt.h>
#include <mm.h>
#include <atomic.h>

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

struct kmessage {
  void * buffer;
  size_t size;
  size_t reply_size;
  class Thread * volatile thread;
  u32_t flags;

  u32_t  a0;
  u32_t  a1;
  u32_t  a2;
  u32_t  a3;
};

#define MAX_MSG_COUNT 32

#define SYSTID_NAMER   1
#define SYSTID_PROCMAN 2

#define SIGNAL_ALARM   0

#define _MSG_SENDER_ANY    0
#define _MSG_SENDER_SIGNAL 1

#define _MSG_SYSTEM_TIDS 0x1000

inline bool SYSTEM_TID(tid_t tid)
{
  return tid < _MSG_SYSTEM_TIDS;
}

class Thread{
 private:
  off_t stack_pl0;
  u32_t alarm;
  u32_t signals;
  
 public:
  Thread(class TProcess *process,
	 off_t eip,
	 u16_t flags,
	 void * kernel_stack,
	 void * user_stack,
	 u16_t code_segment=USER_CODE_SEGMENT,
	 u16_t data_segment=USER_DATA_SEGMENT);

  ~Thread();
  void run();

  class TProcess *process;
  struct TSS *tss;
  gdt_entry descr;
  u16_t flags;
  tid_t send_to;
  void set_tss(register off_t eip,
	       register void *kernel_stack,
	       register void *user_stack,
	       u16_t code_segment=USER_CODE_SEGMENT,
	       u16_t data_segment=USER_DATA_SEGMENT);

  List<kmessage *> *new_messages;
  atomic_t new_messages_count;
  List<kmessage *> *received_messages;
  atomic_t received_messages_count;

  inline void set_signal(u8_t signal_n)
  {
    signals |= 1 << signal_n;
  }

  inline u32_t get_signals()
  {
    return signals;
  }
  
  inline u32_t get_alarm()
  {
    return alarm;
  }
  
  inline void set_alarm(u32_t time)
  {
    alarm = time;
  }
  
  void parse_signals();
  res_t put_message(kmessage *message);
};

class TProcess {
 public:
  TProcess();
  ~TProcess();
  void run();

  List<Thread *> *threads;
  Memory *memory;
  u32_t LoadELF(register void *image);
  string name;

  Thread *thread_create(off_t eip,
			u16_t flags,
			void * kernel_stack,
			void * user_stack,
			u16_t code_segment=USER_CODE_SEGMENT,
			u16_t data_segment=USER_DATA_SEGMENT);
};

#define MESSAGE_ASYNC 1

#endif
