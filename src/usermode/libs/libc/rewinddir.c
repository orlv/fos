/* 
  Copyright (C) 2007 Sergey Gridassov
 */
#include <dirent.h>
#include <fos/fs.h>

void rewinddir(DIR *dir) {
	if(!dir) return;

	struct dirfd *fd = (struct dirfd *) dir;
	fd->offset = 0;
}	
