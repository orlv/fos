/*
 * Copyright (C) 2007 Serge Gridassov
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fos/fos.h>
#include <string.h>

#define ce(src)  ((((src >> 0) & 0xFF) << 24) | (((src >> 8) & 0xFF) << 16) | (((src >> 16) & 0xFF) << 8) | ((src >> 24) & 0xFF))

typedef struct {
  u32_t version;
  u32_t numstr;
  u32_t longlen;
  u32_t shortlen;
  u32_t flags;
  char delim[4];
} str_t;

char *index;
char *data;
str_t *hdr;
off_t *seek;

int main(int argc, char *argv[])
{
  if (argc < 3) {
    printf("Usage: %s <string file> <data file>\n", argv[0]);
    return 1;
  }
  int hndl = open(argv[2], 0);

  if (!hndl) {
    printf("Failed loading data file\n");
    return 1;
  }
  int size = lseek(hndl, 0, SEEK_END);
  lseek(hndl, 0, SEEK_END);

  index = malloc(size);
  read(hndl, index, size);
  close(hndl);

  hdr = (str_t *) index;

  seek = (off_t *) (index + sizeof(str_t) + 8);
  srandom(uptime());
  int selected = random() % ce(hdr->numstr);
  off_t sk = ce(seek[selected * 2 + 1]);

  hndl = open(argv[1], 0);
  if (!hndl) {
    printf("Failed loading data\n");
    return 1;
  }
  size = lseek(hndl, 0, SEEK_END);
  lseek(hndl, 0, SEEK_END);
  size -= sk;
  data = malloc(size);
  lseek(hndl, sk, SEEK_SET);
  read(hndl, data, size);
  close(hndl);
  char *dat = data;
  char *quote = strsep(&data, "%");

  printf("%s\n", quote);
  free(dat);
  free(index);
  return 0;
}
