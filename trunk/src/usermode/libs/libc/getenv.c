/*
 * from dietlibc
 */
#include <string.h>
#include <unistd.h>

char *getenv(const char *s) {
	if(!environ || !s) 
		return NULL;

	size_t len = strlen(s);

	for(int i = 0; environ[i]; i++) 
		if((strncmp(environ[i], s, len) == 0) && (environ[i][len] == '='))
			return environ[i] + len + 1;

	return 0;
}
