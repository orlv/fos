/*
  startup.c
  Copyright (C) 2007 Oleg Fedorov
*/

int main(void);

#include <fos.h>
#include <fs.h>

void _start()
{
  main();
  while(1);
}
