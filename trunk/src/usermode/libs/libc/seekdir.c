/* 
  Copyright (C) 2007 Sergey Gridassov
 */
#include <dirent.h>
#include <fos/fs.h>

void seekdir(DIR *dir, off_t offset) {
	struct dirfd *fd = (struct dirfd *) dir;
	fd->offset = offset;
}
