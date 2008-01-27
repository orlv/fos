#include <string.h>
#include <stdlib.h>
#include <errno.h>

char *getcwd(char *buf, size_t size) {
	if(!size || !buf) {
		errno = EINVAL;
		return NULL;
        }
	char *var = getenv("PWD");

	if(!var) {
		errno = EACCES;
		return NULL;
	}

	if(strlen(var) < size) {
		errno = ERANGE;
		return NULL;
	}

	strncpy(buf, var, size);
	return buf;
}
