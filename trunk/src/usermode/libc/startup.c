/*
  startup.c
  Copyright (C) 2007 Oleg Fedorov
*/

int main(void);
void exit(void);

void _start()
{
  main();
  exit();
}
