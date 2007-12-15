/* 
  Copyright (C) 2007 Sergey Gridassov
 */
#include <dirent.h>
#include <fos/fs.h>

off_t telldir(DIR *dir) {
	struct dirfd *fd = (struct dirfd *) dir;
	return fd->offset;
}
