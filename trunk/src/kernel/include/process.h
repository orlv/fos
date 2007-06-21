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

struct message {
  void *send_buf;
  size_t send_size;
  void *recv_buf;
  size_t recv_size;
  tid_t tid;
} __attribute__ ((packed));

struct kmessage {
  void * send_buf;
  size_t send_size;
  void * recv_buf;
  size_t recv_size;
  class Thread * thread;
  u32_t flags;
};

class Thread{
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
  void set_stack_pl0();
  void set_tss(register off_t eip,
	       register void *kernel_stack,
	       register void *user_stack,
	       u16_t code_segment=USER_CODE_SEGMENT,
	       u16_t data_segment=USER_DATA_SEGMENT);
  void kprocess_set_tss(register off_t eip);
  List<kmessage *> *new_msg;
  List<kmessage *> *recvd_msg;
};

class TProcess {
 private:
  u32_t LoadELF(register void *image);

 public:
  TProcess();
  ~TProcess();
  void run();

  List<Thread *> *threads;
  Memory *memory;

  Thread *thread_create(off_t eip,
			u16_t flags,
			void * kernel_stack,
			void * user_stack,
			u16_t code_segment=USER_CODE_SEGMENT,
			u16_t data_segment=USER_DATA_SEGMENT);
};

#define MESSAGE_ASYNC 1


#endif
