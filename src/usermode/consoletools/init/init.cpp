/*
  Copyright (C) 2007 Serge Gridassov
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

void ParseLine(char *line);

asmlinkage int main()
{
  for (int i = 0; i < 15; i++)
    sched_yield();

  while (!exec("/mnt/modules/tty", NULL)) ;

  printf("Init started! If you see this text - all work fine.\n");
  exec("/mnt/modules/romfs", NULL);

  for (int i = 0; i < 50; i++)
    sched_yield();

  int hndl = open("/etc/inittab", 0);

  printf("Parsing config\n");
  struct stat st;

  fstat(hndl, &st);
  char *config = new char[st.st_size];
  char *cfg = config;

  read(hndl, config, st.st_size);
  close(hndl);
  for (char *ptr = strsep(&config, "\n"); ptr; ptr = strsep(&config, "\n")) {
    if (ptr[0] == '#')
      continue;			// комментарии
    ParseLine(ptr);
  }
  delete cfg;

  printf("All started up.\n");
  return 0;
}

void ParseLine(char *line)
{
  char *tmp = new char[strlen(line)];

  strcpy(tmp, line);
  char *original = tmp;
  char *tokens[3];
  int i = 0;

  for (char *ptr = strsep(&tmp, ":"); ptr && i < 3; ptr = strsep(&tmp, ":"), i++)
    tokens[i] = ptr;
  if (i < 3) {
    delete original;

    return;
  }
  if (strcmp(tokens[2], " "))
    printf("%s\n", tokens[2]);
  if (strcmp(tokens[1], " "))
    exec(tokens[0], tokens[1]);
  else
    exec(tokens[0], NULL);

  delete original;
}
