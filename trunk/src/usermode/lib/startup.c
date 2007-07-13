/*
  startup.c
  Copyright (C) 2007 Oleg Fedorov
*/

int main(void);

void _start()
{
  main();
  while(1);
}
