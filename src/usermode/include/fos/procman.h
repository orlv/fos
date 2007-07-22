/*
  include/fos/procman.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_PROCMAN_H
#define _FOS_PROCMAN_H

#include <types.h>
#include <fos/fs.h>

#define PROCMAN_CMD_EXEC             (BASE_CMD_N + 0)
#define PROCMAN_CMD_KILL             (BASE_CMD_N + 1)
#define PROCMAN_CMD_EXIT             (BASE_CMD_N + 2)
#define PROCMAN_CMD_THREAD_EXIT      (BASE_CMD_N + 3)
#define PROCMAN_CMD_MEM_ALLOC        (BASE_CMD_N + 4)
#define PROCMAN_CMD_MEM_MAP          (BASE_CMD_N + 5)
#define PROCMAN_CMD_MEM_FREE         (BASE_CMD_N + 6)
#define PROCMAN_CMD_CREATE_THREAD    (BASE_CMD_N + 7)
#define PROCMAN_CMD_INTERRUPT_ATTACH (BASE_CMD_N + 8)
#define PROCMAN_CMD_INTERRUPT_DETACH (BASE_CMD_N + 9)
#define PROCMAN_CMD_DMESG            (BASE_CMD_N + 10)


asmlinkage u32_t kill(tid_t tid);
asmlinkage tid_t exec(const char * filename);

asmlinkage void * kmemmap(offs_t ptr, size_t size);

#define MEM_FLAG_LOWPAGE 1

asmlinkage void * kmalloc(size_t size, u32_t flags);
asmlinkage int kfree(off_t ptr);

asmlinkage tid_t thread_create(off_t eip);

asmlinkage int resmgr_attach(const char *pathname);

asmlinkage size_t dmesg(char *buf, size_t count);

#endif
