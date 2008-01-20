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
#include <fos/signal.h>

#define _MSG_SYSTEM_TIDS 0x1000

inline bool SYSTEM_TID(tid_t tid)
{
  return tid < _MSG_SYSTEM_TIDS;
}

class Thread {
 private:
  off_t stack_pl0;

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

  struct {
    struct {
      List<kmessage *> list;
      atomic_t count;
    } unread;

    struct {
      List<kmessage *> list;
      atomic_t count;
    } read;
  } messages;
  res_t put_message(kmessage *message);
  
  struct {
    u32_t time;
    inline u32_t get() {
      return time;
    };
    inline void set(u32_t time) {
      this->time = time;
    };
  } alarm;
  
  List<signal *> signals;
  atomic_t signals_cnt;
  inline void put_signal(u32_t data, u32_t n){
    signal *sig = new signal;
    sig->data = data;
    sig->n = n;
    signals.add_tail(sig);
    signals_cnt.inc();
  }
  void parse_signals();
};

#endif
