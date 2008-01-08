#include <stdlib.h>
#include <stdio.h>
asmlinkage int main(int argc, char **argv)
{
	printf("Realloc test\n");
	printf("Filling first part\n");
	char *buf = realloc(NULL, 13);
	for(int i = 0; i < 13; i++)
		buf[i] = 'A' + i;
	printf("And dump:\n");
	for(int i = 0; i < 13; i++)
		printf("%c", buf[i]);

	printf("\n");
	printf("Reallocating\n");
	buf = realloc(buf, 26);
	printf("Filling second part\n");
	for(int i = 13; i < 26; i++)
		buf[i] = 'A' + i;	

	printf("And dump:\n");

	for(int i = 0; i < 26; i++)
		printf("%c", buf[i]);

	printf("\nReallocating\n");
	buf = realloc(buf, 13);	
	printf("And dump:\n");

	for(int i = 0; i < 13; i++)
		printf("%c", buf[i]);
	free(buf);
	return 0;
}
