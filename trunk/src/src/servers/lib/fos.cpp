/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>

#define PROCMAN_CMD_EXEC      0
#define PROCMAN_CMD_KILL      1
#define PROCMAN_CMD_EXIT      2
#define PROCMAN_CMD_MEM_ALLOC 3
#define PROCMAN_CMD_MEM_MAP   4

#define PID_PROCMAN 0

struct procman_message {
  u32_t cmd;
  union {
    char buf[252];
    u32_t pid;
    u32_t value;
    struct {
      u32_t a1;
      u32_t a2;
    }val;
  }arg;
} __attribute__ ((packed));

asmlinkage void exit()
{
  struct msg *msg = new struct msg;
  struct procman_message *pm = new procman_message;
  pm->cmd = PROCMAN_CMD_EXIT;

  msg->send_buf = pm;
  msg->recv_buf = 0;
  msg->send_size = sizeof(struct procman_message);
  msg->recv_size = 0;
  msg->pid = PID_PROCMAN;
  send(msg);

  while(1);
}

asmlinkage void kill(pid_t pid)
{
  struct msg *msg = new struct msg;
  struct procman_message *pm = new procman_message;
  pm->cmd = PROCMAN_CMD_KILL;
  pm->arg.pid = pid;

  msg->send_buf = pm;
  msg->recv_buf = 0;
  msg->send_size = sizeof(struct procman_message);
  msg->recv_size = 0;
  msg->pid = PID_PROCMAN;
  send(msg);
  delete msg;
  delete pm;
}

asmlinkage res_t exec(string filename)
{
  int i = strlen(filename);
  if(i >= 256)
    return RES_FAULT;

  char res[3];
  res[2] = 0;
  struct msg *msg = new struct msg;
  struct procman_message *pm = new struct procman_message;
  
  pm->cmd = PROCMAN_CMD_EXEC;
  strcpy(pm->arg.buf, filename);

  msg->send_buf = pm;
  msg->recv_buf = res;
  msg->send_size = sizeof(struct procman_message);
  msg->recv_size = 2;
  msg->pid = PID_PROCMAN;
  send(msg);

  delete msg;
  delete pm;

  if(strcpy(res, "OK"))
    return RES_SUCCESS;
  else
    return RES_FAULT;
}

struct msg __msg;
struct procman_message __pm;
u32_t __res;

asmlinkage void *kmemmap(offs_t ptr, size_t size)
{
  struct msg *msg = &__msg;
  struct procman_message *pm = &__pm;

  pm->cmd = PROCMAN_CMD_MEM_MAP;
  pm->arg.val.a1 = ptr;
  pm->arg.val.a2 = size;
    
  msg->send_buf = pm;
  msg->recv_buf = &__res;
  msg->send_size = sizeof(struct procman_message);
  msg->recv_size = sizeof(__res);
  msg->pid = PID_PROCMAN;
  send(msg);

  return (void *)__res;
}

asmlinkage void *kmalloc(size_t size)
{
  struct msg *msg = &__msg;
  struct procman_message *pm = &__pm;

  pm->cmd = PROCMAN_CMD_MEM_ALLOC;
  pm->arg.value = size;
  msg->send_buf = pm;
  msg->recv_buf = &__res;
  msg->send_size = sizeof(struct procman_message);
  msg->recv_size = sizeof(__res);
  msg->pid = PID_PROCMAN;
  send(msg);

  return (void *)__res;
}
