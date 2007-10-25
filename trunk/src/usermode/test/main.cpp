#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
 #include <unistd.h>

#include <fos/message.h>
#include <sys/mman.h>
#include <string.h>
unsigned short *lfb;
struct mouse_pos {
	int dx;
	int dy;
	int dz;
	int b;
};
asmlinkage int main(int argc, char ** argv)
{
  struct mouse_pos move;
  printf("Mouse test\nPress a button for exit\n");
	int psaux = open("/dev/psaux", 0);
  do {
	read(psaux, &move, sizeof(struct mouse_pos));
        printf("%d, %d\n", move.dx, move.dy);
  } while(!move.b);
  printf("Exiting\n");
  return 0;
}

