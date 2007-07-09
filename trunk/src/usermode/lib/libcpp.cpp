/*
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <malloc.h>

void *operator  new(unsigned int size)
{
  return malloc(size);
}

void *operator  new[] (unsigned int size)
{
  return malloc(size);
}

void operator  delete(void *ptr)
{
  free(ptr);
}

void operator  delete[] (void *ptr)
{
  free(ptr);
}
