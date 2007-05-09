/*
  kernel/main/fs/fs.cpp
  Copyright (C) 2006 Oleg Fedorov
*/

#include <string.h>
#include <stdio.h>
#include <fs.h>
#include <fslayer.h>

string getl(u32_t & start, const string path)
{
  u32_t i = start;
  u32_t len;
  while (path[i] == '/')
    i++;
  start = i;
  while (path[i] && (path[i] != '/'))
    i++;
  if (!(len = i - start))
    return 0;

  string name = new char[len + 1];
  string ptr = path;
  ptr += start;
  start += len;

  strncpy(name, ptr, len);
  name[len] = '\0';
  return name;
}

string *createmas(u32_t & n, const string path)
{
  string *mas = new string[1024];
  u32_t start = 0;
  for (n = 0; n < 1024; n++) {
    if (!(mas[n] = getl(start, path)))
      break;
  }
  return mas;
}
