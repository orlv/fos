#include <stdio.h>

asmlinkage int main()
{
    printf("Bye, world! ;)\n");
    char *ptr = (char *) 0x9000000;
    *ptr = 0;
    return 0;
}

