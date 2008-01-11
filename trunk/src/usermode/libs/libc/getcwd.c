#include <string.h>
#include <stdlib.h>
char *getcwd(char *buf, size_t size) {
	char *var = getenv("PWD");
	strncpy(buf, var, size);
	return buf;
}
