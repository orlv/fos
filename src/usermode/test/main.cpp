#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fos/message.h>
#include <sys/mman.h>
#include <string.h>
unsigned short *lfb;

asmlinkage int main(int argc, char ** argv)
{
  printf("LFB test\n");
  int tty = open("/dev/tty", 0);
  struct fd *fd = (struct fd *) tty;
  struct message msg;
  sched_yield();
  msg.a0 = 0xffff;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = fd->thread;
  send((struct message *)&msg);
  	if(msg.a2 != NO_ERR) {
		printf("Failed.\n");
		return 1;
	}
 	printf("Video mode %ux%u, lfb @ 0x%x\n", msg.a1, msg.a3, msg.a0);
	lfb = (unsigned short *) kmmap(0, msg.a1 * msg.a3 * 2, 0, msg.a0);
	if(!lfb) {
		printf("Failed.\n");
		return 1;
	}
	printf("LFB mapped to 0x%x\n", lfb);
	printf("Ÿ’œ €Š“ˆ‹‘Ÿ?\n");	
	for(;;) {
		for(u32_t i = 0; i < msg.a1 * msg.a3; i ++) {
			lfb[i]++;
		}
	}
  return 0;
}

