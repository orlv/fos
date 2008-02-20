/*
  Copyright (C) 2008 Oleg Fedorov
 */

#include <stdlib.h>
#include <string.h>

char *strdup(const char *s)
{
  char *r = (char *) malloc(strlen(s) + 1);
  if(r)
    strcpy(r, s);
  return r;
}
