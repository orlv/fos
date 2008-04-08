/*
 * from glibc
 */
#include <string.h>

int __add_to_environ(const char *name, const char *value, const char *combined, int replace);

int setenv(const char *name, const char *value, int replace) {
	if(!name || !*name || strchr (name, '=') != NULL) 
		return -1;
	
	return __add_to_environ(name, value, NULL, replace);
}
