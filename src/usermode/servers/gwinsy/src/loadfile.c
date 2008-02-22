/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void *load_file(char *filename)
{
  int hndl = open(filename, 0);

  if (!hndl)
    return NULL;

  int size = lseek(hndl, 0, SEEK_END);
  lseek(hndl, 0, SEEK_SET);

  void *buf = malloc(size);

  if (!buf)
    return NULL;
  if (!read(hndl, buf, size)) {
    free(buf);
    return NULL;
  }
  close(hndl);
  return buf;
}
