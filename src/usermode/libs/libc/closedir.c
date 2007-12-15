#include <fos/fs.h>
#include <unistd.h>
#include <fos/message.h>
#include <stdlib.h>
int closedir(DIR *dir)
{
  if(!dir)
    return -1;

  struct dirfd *fd = (struct dirfd *) dir;
  
  struct message msg;
  msg.arg[0] = FS_CMD_DIRCLOSE;
  msg.arg[1] = fd->inode;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = fd->thread;
  
  free(fd->fullname);
  free(fd);

  do_send(&msg);
  
  return 0;
}
