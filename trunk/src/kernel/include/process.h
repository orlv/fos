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
  void * volatile send_buf;
  size_t volatile send_size;
  void * volatile recv_buf;
  size_t volatile recv_size;
  class Thread * volatile thread;
  u32_t flags;
};

#define MAX_MSG_COUNT 32

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
  void set_tss(register off_t eip,
	       register void *kernel_stack,
	       register void *user_stack,
	       u16_t code_segment=USER_CODE_SEGMENT,
	       u16_t data_segment=USER_DATA_SEGMENT);

  List<kmessage *> *new_msg;
  volatile size_t newmsg_count;
  List<kmessage *> *recvd_msg;
  volatile size_t recvdmsg_count;
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
