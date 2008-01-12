/*
  fos/process.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _FOS_PROCESS_H
#define _FOS_PROCESS_H

#include <types.h>
#include <fos/thread.h>

class TProcess {
 public:
  TProcess();
  ~TProcess();
  void run();

  List<Thread *> *threads;
  List<Thread *> *waiting;
  VMM *memory;
  u32_t LoadELF(register void *image);
  string name;

  Thread *thread_create(off_t eip,
			u16_t flags,
			void * kernel_stack,
			void * user_stack,
			u16_t code_segment=USER_CODE_SEGMENT,
			u16_t data_segment=USER_DATA_SEGMENT);
};

#endif
