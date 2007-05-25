/*
  fs/parse.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <string.h>
#include <list.h>

size_t p_len(string p)
{
  size_t i = 0;
  while (p[i] && (p[i] != '/'))
    i++;
  return i;
}

List * path_strip(const string path)
{
  string name;
  size_t len;
  string p = path;
  List *lpath = 0;

  while(1){
    while (*p == '/') p++;
    
    if((len = p_len(p))){
      name = new char[len + 1];
      strncpy(name, p, len);
      if(lpath)
	lpath->add_tail(name);
      else
	lpath = new List(name);
      p += len;
    }
    else break;
  }

  return lpath;
}
