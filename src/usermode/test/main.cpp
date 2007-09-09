#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

asmlinkage int main(int argc, char ** argv)
{
  printf("argc=%d\n", argc);
  for(int i=0; i<argc; i++)
    printf("arg%d = [%s]\n", i, argv[i]);
  printf("test done!\n");
  return 0;
}

