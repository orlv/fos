/*
  fos/thread.h
  Copyright (C) 2005-2008 Oleg Fedorov
*/

#ifndef _FOS_THREAD_H
#define _FOS_THREAD_H

#include <types.h>
#include <fos/tss.h>
#include <fos/gdt.h>
#include <fos/mm.h>
#include <fos/messenger.h>
#include <c++/list.h>
#include <mutex.h>
#include <fos/signal.h>
#include <fos/preempt.h>

#define TSTATE_WAIT_ON_SEND  0x01 /* ожидает при отправке сообщения */
#define TSTATE_WAIT_ON_RECV  0x02 /* ожидает при получении сообщения */

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

  List<Thread *> *me;

  class TProcess *process; /* процесс, в рамках которого запущена нить */
  struct TSS *tss;

  tid_t tid;
  gdt_entry descr;

  u32_t state; /* состояние выполнения потока */
  mutex_t starting; /* захватываем перед изменением состояния */
  void activate();
  void deactivate();

  inline void start(u32_t flag){
    if(state) {
      if(mutex_try_lock(&starting)){
	state &= ~flag;
	if(!state)
	  activate();
	mutex_unlock(&starting);
      }
    }
  }

  inline void wait(u32_t flag){
    preempt_disable();
    deactivate();
    state |= flag;
    sched_yield();
  }

  inline void wait_no_resched(u32_t flag){
    preempt_disable();
    deactivate();
    state |= flag;
  }
  
  u32_t flags;
  tid_t send_to; /* при отправке сообщения, здесь указывается адресат */
  void set_tss(register off_t eip,
	       register void *kernel_stack,
	       register void *user_stack,
	       u16_t code_segment=USER_CODE_SEGMENT,
	       u16_t data_segment=USER_DATA_SEGMENT);

  Messenger messages;

  struct {
    u32_t time;
    List<Thread *> *timer;
  } alarm;
  
  List<signal *> signals;
  atomic_t signals_cnt;
  inline void put_signal(u32_t data, u32_t n){
    signal *sig = new signal;
    sig->data = data;
    sig->n = n;
    signals.add_tail(sig);
    signals_cnt.inc();
    start(TSTATE_WAIT_ON_RECV);
  }
  void parse_signals();
};

#endif
