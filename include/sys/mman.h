#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <types.h>

asmlinkage void *kmmap(void *start, size_t lenght, int flags, off_t phys_start);
asmlinkage int kmunmap(off_t start, size_t lenght);

#endif
