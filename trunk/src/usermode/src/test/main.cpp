#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

asmlinkage int main()
{
  struct stat *buf = new struct stat;
  //int fd = open("/mnt/modules/test.txt", 0);
  //fstat(fd, buf);
  stat("/mnt/modules/test.txt", buf);

  printf("size=%d\n", buf->st_size);

  printf("Bye, world! ;)\n");
  //char *ptr = (char *) 0x9000000;
  //*ptr = 0;
  return 0;
}

