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
  /*
  while(!exec("/mnt/modules/stty", NULL));

  printf("Init started! If you see this text - all work fine.\n");
  */
  setenv("PATH", "/bin:/usr/bin", 0);
  setenv("PWD", "/", 0);

  exec("/mnt/modules/romfs", NULL);

  for (int i = 0; i < 8000; i++)
    sched_yield();

//  printf("init: bootstrapped tty & romfs, reading config\n");

  int hndl = open("/etc/inittab", 0);
  if(!hndl) {
//	printf("init: Fatal error, not found config file.\n");
	return 1;
  }

  int size = lseek(hndl, 0, SEEK_END);
  lseek(hndl, 0, SEEK_SET);

  char *config = new char[size + 1];

  read(hndl, config, size);
  close(hndl);
  for (char *ptr = strsep(&config, "\n"); ptr; ptr = strsep(&config, "\n")) {
    while(ptr[0] == ' ' || ptr[0] == '\t') ptr++; // пробелы в начале

    if (ptr[0] == '#' || ptr[0] == 0)
      continue;			// комментарии и пустые строки
    ParseLine(ptr);
  }

//  printf("All started up.\n");
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

  if(!strcmp(tokens[0], ".")) {
    if(!strcmp(tokens[1], "open_tty")) {
      char *tty = new char[strlen(tokens[2]) + 1];
      strcpy(tty, tokens[2]);
      setenv("STDOUT", tty, 0);
     do {
        stdout = fopen(tty, "w");
      } while(!stdout);
    } else if(!strcmp(tokens[1], "open_in")) {
      char *tty = new char[strlen(tokens[2]) + 1];
      strcpy(tty, tokens[2]);
      setenv("STDIN", tty, 0);
     do {
        stdin = fopen(tty, "w");
      } while(!stdin);
      fclose(stdin);
    }
    return;
  }

  if(tokens[2][0] != 0)
    printf("%s\n", tokens[2]);
  exec(tokens[0], tokens[1]);

}

