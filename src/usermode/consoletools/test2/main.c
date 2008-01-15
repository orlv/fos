#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
static const char testdata[] = "it is test!\nHello, World of tmpfs!\n";
asmlinkage int main(int argc, char **argv)
{
	int hndl = open("/tmp/file", O_CREAT);
	write(hndl, testdata, sizeof(testdata));
	close(hndl);
	printf("Created /tmp/file!\n");
	return 0;
}
