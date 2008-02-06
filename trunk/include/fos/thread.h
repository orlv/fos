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

#define WFLAG_SEND   0x01
#define WFLAG_RECV   0x02
#define WFLAG_SIGNAL 0x02
#define WFLAG_KILL   0x04

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
  u32_t wflags; /* флаги ожидания */
  u32_t flags;
  tid_t send_to; /* при отправке сообщения, здесь указывается адресат */
  void set_tss(register off_t eip,
	       register void *kernel_stack,
	       register void *user_stack,
	       u16_t code_segment=USER_CODE_SEGMENT,
	       u16_t data_segment=USER_DATA_SEGMENT);

  struct {
    struct {
      List<kmessage *> list;
      atomic_t count;

      inline List<kmessage *> *get(Thread *sender, u32_t flags){
	List<kmessage *> *entry;
	/* пройдём по списку в поисках нужного сообщения */
	list_for_each (entry, list) {
	if(entry->item->thread == sender)
	  return entry;
	}
	/* сообщение не найдено, и не придёт */
	if(sender && count.value() > MAX_MSG_COUNT)
	  return -1;
	
	/* сообщение не найдено, но можно повторить попытку позже */
	return 0;
      }
    } unread;

    struct {
      List<kmessage *> list;
      atomic_t count;
    } read;

    /* находит сообщение в списке непрочтённых */
    inline kmessage *get(Thread *sender, u32_t flags) {
      kmessage *msg;
      List<kmessage *> *entry;

      if((from) || (flags & MSG_ASYNC)) {
	entry = unread.get(sender, flags);
      } else { /* любое сообщение */
	entry = (unread.count.value())?(unread.list.next):(0);
      }

      if(!entry || (entry == -1)) return (kmessage *)entry; /* возврат ошибки */

      msg = entry->item;
      if (!(msg->flags & MSG_ASYNC)){ /* перемещаем сообщение в очередь полученных сообщений */
	entry->move_tail(&read.list);
	read.count.inc();
      } else /* убираем сообщение из списков */
	delete entry; 

      unread.count.dec();
      return msg;
    }
  } messages;

  /*  inline void wait_message(){
    wait(WFLAG_RECV);
    sched_yield();
    }*/

  res_t put_message(kmessage *message);
  
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
    start(WFLAG_SIGNAL);
  }
  void parse_signals();
};

#endif
