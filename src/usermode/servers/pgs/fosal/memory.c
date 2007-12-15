#include <sys/mman.h>

void *RequestMemory(int c)
{
  return (void *)kmmap(0, c, 0, 0);
}
