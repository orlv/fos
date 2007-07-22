#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <types.h>
#include <config.h>


#define NULL 0
#define getpagesize()     (PAGE_SIZE)

//asmlinkage int open(const char *pathname, int flags);
asmlinkage int close(int fildes);
asmlinkage ssize_t read(int fildes, void *buf, size_t bytes);
asmlinkage ssize_t write(int fildes, const void *buf, size_t bytes);
asmlinkage off_t lseek(int fildes, off_t offset, int whence);

//extern void *sbrk(ssize_t incr);
//extern void usleep(unsigned long usec);
//extern unsigned int sleep(unsigned int seconds);

#endif
