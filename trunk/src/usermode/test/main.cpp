#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fos/message.h>
#include <sys/mman.h>
#include <string.h>

asmlinkage int main(int argc, char ** argv)
{
  int tty = open("/dev/fb", 0);
  struct fd *fd = (struct fd *) tty;
  
  struct message volatile msg;
  volatile char *buffer = (volatile char *) kmmap(0, 4096, 0, 0);
  strcpy((char *)buffer, "hello");
  
  msg.a0 = 666;
  msg.send_buf = (char *)buffer;
  msg.send_size = 4096;
  msg.recv_size = 0;
  msg.flags = MSG_MEM_SHARE;
  msg.tid = fd->thread;

  send((struct message *)&msg);
  printf("rep\n");
  u8_t i=0;
  while(1) {
    buffer[0] = i;
    i++;
  }
  return 0;
}

