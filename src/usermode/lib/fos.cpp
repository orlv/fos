/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <fs.h>

#define PROCMAN_CMD_EXEC             0
#define PROCMAN_CMD_KILL             1
#define PROCMAN_CMD_EXIT             2
#define PROCMAN_CMD_MEM_ALLOC        3
#define PROCMAN_CMD_MEM_MAP          4
#define PROCMAN_CMD_CREATE_THREAD    5
#define PROCMAN_CMD_INTERRUPT_ATTACH 6
#define PROCMAN_CMD_INTERRUPT_DETACH 7
#define PROCMAN_CMD_DMESG            8

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
  message msg;
  u32_t res;
  fs_message m;
  msg.recv_size = sizeof(res);
  msg.recv_buf = &res;
  msg.send_size = sizeof(fs_message);
  m.data3.cmd = NAMER_CMD_RESOLVE;
  strcpy((char *)m.data3.buf, name);
  msg.send_buf = (char *)&m;
  msg.tid = 0;
  send(&msg);
  return res;
}

extern tid_t procman;
extern tid_t namer;

asmlinkage void exit()
{
  message msg;
  procman_message pm;
  pm.cmd = PROCMAN_CMD_EXIT;

  msg.send_buf = (char *)&pm;
  msg.recv_buf = 0;
  msg.send_size = sizeof(procman_message);
  msg.recv_size = 0;
  msg.tid = procman;
  send(&msg);

  while(1);
}

asmlinkage void kill(tid_t tid)
{
  message msg;
  procman_message pm;
  pm.cmd = PROCMAN_CMD_KILL;
  pm.arg.tid = tid;

  msg.send_buf = (char *)&pm;
  msg.recv_buf = 0;
  msg.send_size = sizeof(procman_message);
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
  message msg;
  procman_message pm;
  
  pm.cmd = PROCMAN_CMD_EXEC;
  strcpy(pm.arg.buf, filename);

  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(procman_message);
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
  message msg;
  procman_message pm;
  volatile u32_t res;

  pm.cmd = PROCMAN_CMD_MEM_MAP;
  pm.arg.val.a1 = ptr;
  pm.arg.val.a2 = size;
    
  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(procman_message);
  msg.recv_size = sizeof(res);
  msg.tid = procman;
  send(&msg);

  return (void *)res;
}

asmlinkage void *kmalloc(size_t size)
{
  message msg;
  procman_message pm;
  volatile u32_t res;
  
  pm.cmd = PROCMAN_CMD_MEM_ALLOC;
  pm.arg.value = size;
  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(procman_message);
  msg.recv_size = sizeof(res);
  msg.tid = procman;
  send(&msg);

  return (void *)res;
}

asmlinkage tid_t thread_create(off_t eip)
{
  message msg;
  procman_message pm;
  volatile u32_t res;
  
  pm.cmd = PROCMAN_CMD_CREATE_THREAD;
  pm.arg.value = eip;
  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(procman_message);
  msg.recv_size = sizeof(res);
  msg.tid = procman;
  send(&msg);

  return (tid_t)res;
}

/*
  при возникновении указанного прерывания данному потоку
  будет приходить сообщение
*/
asmlinkage res_t interrupt_attach(u8_t n)
{
  message msg;
  procman_message pm;
  volatile u32_t res;
  
  pm.cmd = PROCMAN_CMD_INTERRUPT_ATTACH;
  pm.arg.value = n;
  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(procman_message);
  msg.recv_size = sizeof(res);
  msg.tid = procman;
  send(&msg);

  return res;
}

asmlinkage res_t interrupt_detach(u8_t n)
{
  message msg;
  procman_message pm;
  volatile u32_t res;
  
  pm.cmd = PROCMAN_CMD_INTERRUPT_DETACH;
  pm.arg.value = n;
  msg.send_buf = (char *)&pm;
  msg.recv_buf = (char *)&res;
  msg.send_size = sizeof(procman_message);
  msg.recv_size = sizeof(res);
  msg.tid = procman;
  send(&msg);

  return res;
}

asmlinkage int resmgr_attach(const char *pathname)
{
  if(!pathname)
    return 0;

  message msg;
  fs_message m;
  int res;
  msg.recv_size = sizeof(res);
  msg.recv_buf = &res;
  size_t len = strlen(pathname);
  msg.send_size = 8 + len + 4;
  m.data3.cmd = NAMER_CMD_ADD;
  strncpy((char *)m.data3.buf, pathname, len);
  m.data3.buf[len] = 0;
  msg.send_buf = &m;
  msg.tid = PID_NAMER;
  send(&msg);
  return res;
}

asmlinkage size_t read(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  fs_message m;
  msg.recv_size = count;
  msg.recv_buf = buf;
  msg.send_size = 4;
  m.data.cmd = FS_CMD_READ;
  msg.send_buf = (char *)&m;
  msg.tid = fd->thread;
  send(&msg);
  return msg.recv_size;
}

asmlinkage size_t write(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  fs_message m;
  msg.recv_size = count;
  msg.recv_buf = buf;

  if(count > FS_CMD_LEN)
    msg.send_size = sizeof(fs_message);
  else
    msg.send_size = 8 + count;
  
  m.data.cmd = FS_CMD_WRITE;
  msg.send_buf = (char *)&m;
  msg.tid = fd->thread;
  send(&msg);
  return msg.send_size - 8;
}

asmlinkage fd_t open(const char *pathname, int flags)
{
  tid_t thread = resolve("/dev/keyboard");
  if(!thread)
    return 0;

  struct fd *fd = new struct fd;
  fd->thread = thread;
  return fd;
}

asmlinkage int close(fd_t fd)
{
  if(!fd)
    return -1;

  delete fd;
  return 0;
}

asmlinkage size_t dmesg(char *buf, size_t count)
{
  message msg;
  procman_message pm;

  pm.cmd = PROCMAN_CMD_DMESG;
  msg.send_buf = (char *)&pm;
  msg.recv_buf = buf;
  msg.send_size = sizeof(procman_message);
  msg.recv_size = count;
  msg.tid = procman;
  send(&msg);

  return msg.recv_size;
}

asmlinkage u32_t uptime()
{
  u32_t time;
  sys_call(_FOS_UPTIME, (u32_t)&time);
  return time;
}