#ifndef _FCNTL_H
#define _FCNTL_H

#include <types.h>

userlinkage int open(const char *pathname, int flags);
userlinkage int open2(const char *pathname, int flags);

#endif
