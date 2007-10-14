#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fos/message.h>

asmlinkage int main(int argc, char ** argv)
{
  int tty = open("/dev/suxx", 0);
  struct fd *fd = (struct fd *) tty;
  
  struct message volatile msg;

  msg.a0 = 666;
  msg.send_buf = "hello";
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = fd->thread;

  u32_t result = send((struct message *)&msg);

  return 0;
}

