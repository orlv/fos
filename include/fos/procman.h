/*
  include/fos/procman.h
  Copyright (C) 2007-2008 Oleg Fedorov
 */

#ifndef _FOS_PROCMAN_H
#define _FOS_PROCMAN_H

#include <types.h>
#include <fos/fs.h>

#define PROCMAN_CMD_EXEC             (BASE_METHOD_N + 0)
#define PROCMAN_CMD_KILL             (BASE_METHOD_N + 1)
#define PROCMAN_CMD_EXIT             (BASE_METHOD_N + 2)
#define PROCMAN_CMD_THREAD_EXIT      (BASE_METHOD_N + 3)
#define PROCMAN_CMD_CREATE_THREAD    (BASE_METHOD_N + 4)
#define PROCMAN_CMD_INTERRUPT_ATTACH (BASE_METHOD_N + 5)
#define PROCMAN_CMD_INTERRUPT_DETACH (BASE_METHOD_N + 6)
#define PROCMAN_CMD_DMESG            (BASE_METHOD_N + 7)

#define MM_CMD_MMAP          (BASE_METHOD_N + 0)
#define MM_CMD_MUNMAP        (BASE_METHOD_N + 1)

#ifdef __KERNEL__

#include <fos/process.h>
#include <c++/list.h>
#include <c++/index.h>

#define FLAG_TSK_KERN         0x02
#define FLAG_TSK_TERM         0x04 /* завершить все потоки в данном адресном пространстве */
#define FLAG_TSK_EXIT_THREAD  0x08 /* завершить только один поток */
#define FLAG_TSK_SYSCALL      0x40 /* поток выполняет системный вызов */

class TProcMan {
 public:
  TProcMan();

  struct {
    List<Thread *> *active; /* готовый к выполнению */
    List<Thread *> *wait;   /* ожидающий события    */
    //List<Thread *> *zombie; /* ожидающий завершения */
    tindex<Thread> *tid;    /* список tid           */
    tindex<TProcess> *pid;  /* список pid           */
  } task;

  struct {
    void check(u32_t uptime) {
      List<Thread *> *curr, *n;
      list_for_each_safe(curr, n, threads){
	if(curr->item->alarm.time <= uptime) {
	  curr->item->alarm.timer = 0;
	  curr->item->start(TSTATE_WAIT_ON_RECV);
	  delete curr;
	}
      }
    }
    inline void add(Thread *thread, u32_t time) {
      thread->alarm.time = time;
      if(!thread->alarm.timer) /* таймера не сущестует - создаём */
	thread->alarm.timer = threads->add_tail(thread);
      else if(!time) { /* таймер существует, а time не задан - удаляем таймер */
	delete thread->alarm.timer;
      }
    }
    List<Thread *> *threads;
  } timer;

  
  tid_t reg_thread(register Thread *thread);
  void unreg_thread(register List<Thread *> *thread);

  inline void activate(List<Thread *> *thread) {
    if(!(thread->item->flags & FLAG_TSK_TERM))
      thread->move(task.active);
  }
  
  inline void stop(List<Thread *> *thread) {
    thread->move_tail(task.wait);
  }
  
  tid_t exec(register void *image, const char *name,
	     const char *args, size_t args_len,
	     const char *envp, size_t envp_len);
  void scheduler();
  List<Thread *> *do_kill(List<Thread *> *thread);
  res_t kill(register tid_t tid, u16_t flag);
  List<Thread *> *curr;
  List<Thread *> *prev;
};

#endif /* __KERNEL__ */
#endif
