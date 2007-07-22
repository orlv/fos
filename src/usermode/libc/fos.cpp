/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <fs.h>

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

asmlinkage void * kmalloc(size_t size, u32_t flags)
{
  message msg;
  msg.a0 = PROCMAN_CMD_MEM_ALLOC;
  msg.a1 = size;
  msg.a2 = flags;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return (void *) msg.a0;
  else
    return 0;
}

asmlinkage int kfree(off_t ptr)
{
  message msg;
  msg.a0 = PROCMAN_CMD_MEM_FREE;
  msg.a1 = ptr;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
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

res_t do_send(message *msg)
{
  res_t result;
  while(1) {
    result = send(msg);
    if(result == RES_FAULT2) { /* очередь получателя переполнена, обратимся чуть позже */
      sched_yield();
      continue;
    }
    return result;
  }
}

asmlinkage ssize_t read(int fildes, void *buf, size_t nbyte)
{
  fd_t fd = (fd_t) fildes;
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  message msg;
  size_t offset = 0;

  if(fd->buf_size < nbyte)
    msg.recv_size = fd->buf_size;
  else
    msg.recv_size = nbyte;

  do{
    msg.a0 = FS_CMD_READ;
    msg.send_size = 0;
    msg.a1 = fd->inode;
    msg.a2 = fd->offset;
    msg.tid = fd->thread;

    msg.recv_buf = &((char *)buf)[offset];

    if(do_send(&msg) != RES_SUCCESS) /* получатель не найден! */
      return -1;

    if(msg.a2 == ERR_UNKNOWN_CMD)
      return -2;
    
    offset += msg.a0;
    fd->offset = offset;
    
    if((msg.a2 == ERR_EOF) || offset >= nbyte)
      return offset;

    if(offset + msg.send_size > nbyte)
      msg.send_size = nbyte - offset;
  } while(1);
}

asmlinkage ssize_t write(int fildes, const void *buf, size_t nbyte)
{
  fd_t fd = (fd_t) fildes;  
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  message msg;
  size_t offset = 0;

  if(fd->buf_size < nbyte)
    msg.send_size = fd->buf_size;
  else
    msg.send_size = nbyte;

  do{
    msg.a0 = FS_CMD_WRITE;
    msg.recv_size = 0;
    msg.a1 = fd->inode;
    msg.a2 = fd->offset;
    msg.tid = fd->thread;

    msg.send_buf = &((char *)buf)[offset];

    if(do_send(&msg) != RES_SUCCESS) /* получатель не найден! */
      return -1;

    if(msg.a2 == ERR_UNKNOWN_CMD)
      return -2;
    
    offset += msg.a0;

    if((msg.a2 == ERR_EOF) || offset >= nbyte)
      return offset;
   
    
    if(offset + msg.send_size > nbyte)
      msg.send_size = nbyte - offset;
  } while(1);
}

asmlinkage off_t lseek(int fildes, off_t offset, int whence)
{
  fd_t fd = (fd_t) fildes;  
  if(!fildes || fildes == -1 || !fd->thread)
    return (off_t)-1;

  switch(whence) {
  case SEEK_SET:
    fd->offset = offset;
    return fd->offset;

  case SEEK_CUR:
    fd->offset += offset;
    return fd->offset;

  case SEEK_END:
  default:
    return (off_t)-1;
  }
}

asmlinkage int open(const char *pathname, int flags)
{
  volatile struct message msg;
  msg.a0 = FS_CMD_ACCESS;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((message *)&msg);
  if(result == RES_SUCCESS && msg.a0 && msg.a2 == NO_ERR) {
    struct fd *fd = new struct fd;
    fd->thread = msg.tid;
    fd->inode = msg.a0;
    fd->buf_size = msg.a1;
    return (int) fd;
  } else
    return -1;
}

asmlinkage int close(int fildes)
{
  if(!fildes || fildes == -1)
    return -1;

  delete (fd_t) fildes;
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

asmlinkage u32_t alarm(u32_t ticks)
{
  return sys_call(_FOS_ALARM, ticks);
}

asmlinkage u32_t alarm2(u32_t ticks)
{
  return sys_call2(_FOS_ALARM, 0, ticks);
}

asmlinkage tid_t my_tid()
{
  return sys_call(_FOS_MYTID, 0);
}
