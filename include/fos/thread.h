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
//#include <fos/system.h>

//#define WFLAG_SEND   0x01
//#define WFLAG_RECV   0x02
//#define WFLAG_SIGNAL 0x02
//#define WFLAG_KILL   0x04

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
  void start(u32_t flag);
  void wait(u32_t flag);

  List<Thread *> *me;

  class TProcess *process; /* процесс, в рамках которого запущена нить */
  struct TSS *tss;

  tid_t tid;
  gdt_entry descr;

  bool wstate;
  bool wflag; /* ожидание сообщения */
  void activate();
  void stop();
  
  inline void start(){
    if(wflag) {
      wflag = 0;
      if(wstate) {
	wstate = 0;
	activate();
      }
    }
  }

  inline void wait(){
    if(wflag) {
      stop();
      wstate = 1;
      extern atomic_t mt_state;
      mt_state.set(0);
      sched_yield();
    }
  }
  
  u32_t flags;
  tid_t send_to; /* при отправке сообщения, здесь указывается адресат */
  void set_tss(register off_t eip,
	       register void *kernel_stack,
	       register void *user_stack,
	       u16_t code_segment=USER_CODE_SEGMENT,
	       u16_t data_segment=USER_DATA_SEGMENT);

  Messenger messages;

  /*  inline void wait_message(){
    wait(WFLAG_RECV);
    sched_yield();
    }*/

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
    start();
  }
  void parse_signals();
};

#endif
