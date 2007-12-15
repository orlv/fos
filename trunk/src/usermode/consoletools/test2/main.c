#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <fos/fos.h>
#include <fos/namer.h>
#include <fos/message.h>

#include <fos/fs.h>

asmlinkage int main(int argc, char **argv)
{
  printf("test2: my TID=0x%X\n", my_tid());

  resmgr_attach("/");

  int fd = open2("/mnt/modules/test.txt", 0);

  if (fd) {
    fd_t foo = (fd_t) fd;

    printf("test2: sid=0x%X, f=[%s], t=[%s]", foo->thread, foo->fullname, foo->name);
  } else
    printf("test2: fd=0");
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
