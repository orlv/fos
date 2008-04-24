/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <string.h>

char *strstr(const char *haystack, const char *needle) {
  if(haystack == NULL || needle == NULL)
    return NULL;

  size_t nl = strlen(needle);
  size_t hl = strlen(haystack);

  if (nl == 0)
    return (char *) haystack;

  if (nl > hl)
    return NULL;

  for (int i = hl - nl + 1; i; i--) {
    if (*haystack == *needle && !memcmp(haystack,needle,nl))
      return (char *) haystack;
    haystack++;
  }
  return 0;
}
