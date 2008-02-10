#ifndef _FCNTL_H
#define _FCNTL_H

#include <types.h>

#define O_FOS_DNTCOPY_NAME 0x00010000

#define O_CREAT		(1 << 0)
#define O_APPEND	(1 << 1)
#define O_EXCL		(1 << 2)
#define O_TRUNC		(1 << 3)
#define O_RDONLY	(1 << 4)
#define O_WRONLY	(1 << 5)

userlinkage int open(const char *pathname, int flags);

#endif
