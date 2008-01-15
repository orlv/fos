#ifndef _FCNTL_H
#define _FCNTL_H

#include <types.h>

#define O_FOS_DNTCOPY_NAME 0x00010000

#define O_CREAT	(1 << 0)
userlinkage int open(const char *pathname, int flags);
//userlinkage int open2(const char *pathname, int flags);

#endif
