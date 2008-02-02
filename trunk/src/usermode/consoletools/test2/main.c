#include <stdio.h>
#include <sched.h>

const char block[] = "Hello, hello, hello fopen-family!\n";

asmlinkage int main(int argc, char **argv)
{
	char buf[10];
	snprintf(buf, 10, "%s\n", block);
	printf("written: %s\n", buf);
	return 0;
}
