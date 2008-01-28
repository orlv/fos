#include <stdio.h>
#include <sched.h>

const char block[] = "Hello, hello, hello fopen-family!\n";

asmlinkage int main(int argc, char **argv)
{
	FILE *f = fopen("/tmp/file", "w");
	for(int i = 0; i < 100; i++)
		fwrite(block, sizeof(block), 1, f);

	fclose(f);
	return 0;
}
