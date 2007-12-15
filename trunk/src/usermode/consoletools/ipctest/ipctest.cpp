/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <sys/mman.h>
#include <string.h>

volatile bool foo = 0;

THREAD(thread)
{
  struct message msg;
  int c = 0;

  foo = 1;
  while (foo) ;

  while (1) {
    msg.recv_size = 0;
    msg.flags = 0;
    msg.tid = 0;
    receive(&msg);

    c++;
    msg.arg[0] = c;
    msg.send_size = 0;
    reply(&msg);
  }
}

asmlinkage int main(int argc, char **argv)
{
  printf("ipctest \n");
  struct message msg;
  tid_t tid = thread_create((off_t) & thread);

  printf("ipctest: tid = 0x%X\n", tid);
  while (!foo) ;
  int t1 = uptime();

  foo = 0;
  u32_t iter = 1012;

  while (1) {
    msg.recv_size = 0;
    msg.send_size = 0;
    msg.flags = 0;
    msg.tid = tid;
    send(&msg);

    if (msg.arg[0] >= iter)
      break;
  }

  int t2 = uptime();

  printf("ipctest: start time %d, end time %d (%d ms), %d iterations\n", t1, t2, t2 - t1, iter);

  return 0;
}
