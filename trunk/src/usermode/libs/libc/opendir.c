/* 
  Copyright (C) 2007 Sergey Gridassov
 */
#include <fos/fs.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fos/message.h>
DIR *opendir(const char *name) {
  if(name[0] != '/') {
    char *pwd = getenv("PWD");
    if(!pwd) return NULL;
    if(pwd[0] != '/') return NULL;
    char *buf = malloc(strlen(name) + strlen(pwd) + 1);
    strcpy(buf, pwd); strcat(buf, name);
    DIR* ret = opendir(buf);
    free(buf);
    return ret;
  }
	struct message msg;
	msg.arg[0] = FS_CMD_DIROPEN;
	size_t len = strlen(name);
	if(len > MAX_PATH_LEN)
		return NULL;

	msg.send_buf = name;
	msg.send_size = len + 1;
	msg.recv_size = 0;
	msg.flags = 0;
	msg.tid = SYSTID_NAMER;

	u32_t result = send((struct message *)&msg);
	if(result == RES_SUCCESS && msg.arg[0] && msg.arg[2] == NO_ERR) {
		struct dirfd *fd = (struct dirfd *) malloc(sizeof(struct dirfd));
		fd->fullname = (const char *) malloc(len + 1);
		strcpy((char *)fd->fullname, name);
		fd->thread = msg.tid;
		fd->inode = msg.arg[0];
		fd->inodes = msg.arg[1];
		return (DIR *) fd;
	} else 
		return NULL;
}
