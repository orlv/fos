/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <fs.h>

#define PROCMAN_CMD_EXEC             (BASE_CMD_N + 0)
#define PROCMAN_CMD_KILL             (BASE_CMD_N + 1)
#define PROCMAN_CMD_EXIT             (BASE_CMD_N + 2)
#define PROCMAN_CMD_MEM_ALLOC        (BASE_CMD_N + 3)
#define PROCMAN_CMD_MEM_MAP          (BASE_CMD_N + 4)
#define PROCMAN_CMD_CREATE_THREAD    (BASE_CMD_N + 5)
#define PROCMAN_CMD_INTERRUPT_ATTACH (BASE_CMD_N + 6)
#define PROCMAN_CMD_INTERRUPT_DETACH (BASE_CMD_N + 7)
#define PROCMAN_CMD_DMESG            (BASE_CMD_N + 8)

asmlinkage void exit()
{
  message msg;
  msg.a0 = PROCMAN_CMD_EXIT;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  send(&msg);
  while(1);
}

asmlinkage u32_t kill(tid_t tid)
{
  message msg;
  msg.a0 = PROCMAN_CMD_KILL;
  msg.a1 = tid;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
  else
    return 0;
}

asmlinkage tid_t exec(const char * filename)
{
  size_t len = strlen(filename);
  if(len+1 > MAX_PATH_LEN)
    return 0;
  message msg;
  msg.a0 = PROCMAN_CMD_EXEC;
  msg.send_buf = filename;
  msg.send_size = len + 1;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;

  if(send(&msg) == RES_SUCCESS)
    return (tid_t) msg.a0;
  else
    return 0;
}

asmlinkage void * kmemmap(offs_t ptr, size_t size)
{
  message msg;
  msg.a0 = PROCMAN_CMD_MEM_MAP;
  msg.a1 = ptr;
  msg.a2 = size;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return (void *) msg.a0;
  else
    return 0;
}

asmlinkage void * kmalloc(size_t size)
{
  message msg;
  msg.a0 = PROCMAN_CMD_MEM_ALLOC;
  msg.a1 = size;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return (void *) msg.a0;
  else
    return 0;
}

asmlinkage tid_t thread_create(off_t eip)
{
  message msg;
  msg.a0 = PROCMAN_CMD_CREATE_THREAD;
  msg.a1 = eip;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return (tid_t) msg.a0;
  else
    return 0;
}

/*
  при возникновении указанного прерывания данному потоку
  будет приходить сообщение
*/
asmlinkage res_t interrupt_attach(u8_t n)
{
  message msg;
  msg.a0 = PROCMAN_CMD_INTERRUPT_ATTACH;
  msg.a1 = n;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
  else
    return 0;
}

asmlinkage res_t interrupt_detach(u8_t n)
{
  message msg;
  msg.a0 = PROCMAN_CMD_INTERRUPT_DETACH;
  msg.a1 = n;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
  else
    return 0;
}

asmlinkage int resmgr_attach(const char *pathname)
{
  if(!pathname)
    return 0;

  message msg;
  msg.a0 = NAMER_CMD_ADD;
  size_t len = strlen(pathname);
  if(len+1 > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.tid = SYSTID_NAMER;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
  else
    return 0;
}

asmlinkage size_t read(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  msg.a0 = FS_CMD_READ;
  msg.recv_size = count;
  msg.recv_buf = buf;
  msg.send_size = 0;
  msg.a1 = fd->id;
  msg.a2 = fd->offset;
  msg.tid = fd->thread;

  do{
    switch(send(&msg)){
    case RES_SUCCESS:
      return msg.recv_size;
      
    case RES_FAULT2: /* очередь получателя переполнена, обратимся чуть позже */
      continue;
      
    default:
      return 0;
    }
  }while(1);
}

asmlinkage size_t write(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  msg.a0 = FS_CMD_WRITE;
  msg.recv_size = 0;
  msg.send_buf = buf;
  msg.a1 = fd->id;
  msg.a2 = fd->offset;
  msg.tid = fd->thread;

  do{
    msg.send_size = count;
    
    switch(send(&msg)){
    case RES_SUCCESS:
      return msg.a0;
      
    case RES_FAULT2: /* очередь получателя переполнена, обратимся чуть позже */
      continue;
      
    default:
      return 0;
    }
  }while(1);
}

asmlinkage fd_t open(const char *pathname, int flags)
{
  volatile struct message msg;
  msg.a0 = FS_CMD_ACCESS;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((message *)&msg);
  if(result == RES_SUCCESS && msg.a0) {
    struct fd *fd = new struct fd;
    fd->thread = msg.tid;
    fd->id = msg.a0;
    return fd;
  } else
    return 0;
}

asmlinkage int close(fd_t fd)
{
  if(!fd)
    return RES_FAULT;

  delete fd;
  return 0;
}

asmlinkage size_t dmesg(char *buf, size_t count)
{
  message msg;
  msg.a0 = PROCMAN_CMD_DMESG;
  msg.recv_buf = buf;
  msg.recv_size = count;
  msg.send_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.recv_size;
  else
    return 0;
}

asmlinkage u32_t uptime()
{
  return sys_call(_FOS_UPTIME, 0);
}

asmlinkage tid_t my_tid()
{
  return sys_call(_FOS_MYTID, 0);
}
