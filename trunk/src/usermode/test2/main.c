#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <fos/fos.h>
#include <fos/namer.h>
#include <fos/message.h>

int resolve(const char *pathname, char **pathtail)
{
  volatile struct message msg;
  msg.arg[0] = NAMER_CMD_RESOLVE;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  char *pt = malloc(len+1);
  
  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_buf = pt;
  msg.recv_size = len+1;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((struct message *)&msg);
  if(result == RES_SUCCESS && msg.arg[0] && msg.arg[2] == NO_ERR) {
    *pathtail = pt;
    return msg.arg[1];
  } else {
    free(pt);
    *pathtail = 0;
    return 0;
  }
}

asmlinkage int main(int argc, char ** argv)
{
  printf("test: my TID=0x%X\n", my_tid());

  resmgr_attach("/");
  
  char *pathtail;
  int sid = resolve("/test.txt", &pathtail);
  if(sid)
    printf("test: sid=0x%X, p=[%s]", sid, pathtail);
  else
    printf("test: sid=0");
  /*
  int fd = open("/mnt/modules/test.txt", 0);
  char *buf = malloc(512);
  int i = read(fd, buf, 512);
  printf("test:read %d bytes\n", i);
  for(int j=0; j<i; j++){
    printf("%c", buf[j]);
  }
  */
  printf("\n");
  return 0;
}

