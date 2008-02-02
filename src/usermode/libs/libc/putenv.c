/*
 * from glibc
 */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
int __add_to_environ(const char *name, const char *value, const char *combined, int replace);
int putenv(char *string) {
	const char *const name_end = strchr(string, '=');
	if(name_end) {
		char *name = __builtin_alloca(name_end - string + 1);
		memcpy(name, string, name_end - string);
		name[name_end - string] = '\0';
		return __add_to_environ (name, NULL, string, 1);
	}
	unsetenv(string);
	return 0;
}
