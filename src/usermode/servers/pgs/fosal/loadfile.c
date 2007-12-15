#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void *load_file(char *filename)
{
  int hndl = open(filename, 0);

  if (!hndl)
    return NULL;
  struct stat st;

  if (fstat(hndl, &st))
    return NULL;
  void *buf = malloc(st.st_size);

  if (!buf)
    return NULL;
  if (!read(hndl, buf, st.st_size)) {
    free(buf);
    return NULL;
  }
  close(hndl);
  return buf;
}
