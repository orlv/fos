/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <fs.h>

#define PROCMAN_CMD_EXEC      0
#define PROCMAN_CMD_KILL      1
#define PROCMAN_CMD_EXIT      2
#define PROCMAN_CMD_MEM_ALLOC 3
#define PROCMAN_CMD_MEM_MAP   4

struct procman_message {
  u32_t cmd;
  union {
    char buf[252];
    u32_t tid;
    u32_t value;
    struct {
      u32_t a1;
      u32_t a2;
    }val;
  }arg;
} __attribute__ ((packed));

asmlinkage tid_t resolve(char *name)
{
  volatile struct message msg;
  u32_t res;
  struct fs_message m;
  msg.recv_size = sizeof(res);
  msg.recv_buf = &res;
  msg.send_size = sizeof(struct fs_message);
  m.cmd = NAMER_CMD_RESOLVE;
  strcpy((char *)m.buf, name);
  msg.send_buf = (char *)&m;
  msg.tid = 0;
  send(&msg);
  return res;
}

tid_t namer;
tid_t tty;
tid_t procman;

asmlinkage void exit()
{
  volatile struct message msg;
  volatile struct procman_message pm;
  pm.cmd = PROCMAN_CMD_EXIT;

  msg.send_buf = (char *)&pm;
  msg.recv_buf = 0;
  msg.send_size = sizeof(struct procman_message);
  msg.recv_size = 0;
  msg.tid = procman;
  send(&msg);

  while(1);
}

asmlinkage void kill(tid_t tid)
{
  volatile struct message msg;
  volatile struct procman_message pm;
  pm.cmd = PROCMAN_CMD_KILL;
  pm.arg.tid = tid;

  msg.send_buf = (char *)&pm;
  msg.recv_buf = 0;
  msg.send_size = sizeof(struct procman_message);
  msg.recv_size = 0;
  msg.tid = procman;
  send(&msg);
}

asmlinkage res_t exec(const string filename)
{
  int i = strlen(filename);
  if(i >= 256)
    return RES_FAULT;

  char res[3];
  res[2] = 0;
  volatile struct message msg;
  struct procman_message pm;
  
  pm.cmd = PROCMAN_CMD_EXEC;
  strcpy(pm.arg.buf, filename);

  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(struct procman_message);
  msg.recv_size = 2;
  msg.tid = procman;
  send(&msg);

  if(strcpy(res, "OK"))
    return RES_SUCCESS;
  else
    return RES_FAULT;
}

asmlinkage void *kmemmap(offs_t ptr, size_t size)
{
  volatile struct message msg;
  volatile struct procman_message pm;
  volatile u32_t res;

  pm.cmd = PROCMAN_CMD_MEM_MAP;
  pm.arg.val.a1 = ptr;
  pm.arg.val.a2 = size;
    
  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(struct procman_message);
  msg.recv_size = sizeof(res);
  msg.tid = procman;
  send(&msg);

  return (void *)res;
}

asmlinkage void *kmalloc(size_t size)
{
  volatile struct message msg;
  volatile struct procman_message pm;
  volatile u32_t res;
  
  pm.cmd = PROCMAN_CMD_MEM_ALLOC;
  pm.arg.value = size;
  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(struct procman_message);
  msg.recv_size = sizeof(res);
  msg.tid = procman;
  send(&msg);

  return (void *)res;
}
