/*
  Copyright (C) 2007 - 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <stdlib.h>
#include <fos/fos.h>
#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

void ParseLine(char *line);

asmlinkage int main()
{

  for (int i = 0; i < 15; i++)
    sched_yield();

  while(!exec("/mnt/modules/stty", NULL));

  do {
    stdout = open("/dev/tty", 0);
  } while(stdout < 0);

  printf("Init started! If you see this text - all work fine.\n");

  setenv("STDOUT", "/dev/tty", 0);
  setenv("STDIN", "/dev/tty", 0);
  setenv("PATH", "/bin:/usr/bin", 0);
  setenv("PWD", "/", 0);

  exec("/mnt/modules/romfs", NULL);

  for (int i = 0; i < 1000; i++)
    sched_yield();

  printf("init: bootstrapped tty & romfs, reading config\n");

  int hndl = open("/etc/inittab", 0);
  if(!hndl) {
	printf("init: Fatal error, not found config file.\n");
	return 1;
  }

  struct stat st;

  fstat(hndl, &st);
  char *config = new char[st.st_size];

  read(hndl, config, st.st_size);
  close(hndl);
  for (char *ptr = strsep(&config, "\n"); ptr; ptr = strsep(&config, "\n")) {
    while(ptr[0] == ' ' || ptr[0] == '\t') ptr++; // пробелы в начале

    if (ptr[0] == '#' || ptr[0] == 0)
      continue;			// комментарии и пустые строки
    ParseLine(ptr);
  }

  printf("All started up.\n");
  return 0;
}

void ParseLine(char *line)
{

  char *tokens[3];
  int i = 0;

  for (char *ptr = strsep(&line, ":"); ptr && i < 3; ptr = strsep(&line, ":"), i++)
    tokens[i] = ptr;
  if (i < 3) 
    return;

  if(tokens[2][0] != 0)
    printf("%s\n", tokens[2]);
  exec(tokens[0], tokens[1]);

}

