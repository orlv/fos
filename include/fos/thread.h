/*
  fos/thread.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _FOS_THREAD_H
#define _FOS_THREAD_H

#include <types.h>
#include <fos/tss.h>
#include <fos/gdt.h>
#include <fos/mm.h>
#include <fos/message.h>
#include <c++/list.h>
#include <c++/atomic.h>


#define _MSG_SYSTEM_TIDS 0x1000

inline bool SYSTEM_TID(tid_t tid)
{
  return tid < _MSG_SYSTEM_TIDS;
}

class Thread {
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

#endif
