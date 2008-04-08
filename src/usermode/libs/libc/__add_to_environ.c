/*
 * from glibc
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static char **last_environ = NULL;
int __add_to_environ(const char *name, const char *value, const char *combined, int replace) {
	register char **ep;
	register size_t size;
	const size_t namelen = strlen(name);
	const size_t vallen = value != NULL ? strlen(value) + 1 : 0;

	ep = environ;

	size = 0;
	if(ep) {
		for(; *ep != NULL; ++ep)
			if(!strncmp(*ep, name, namelen) && (*ep)[namelen] == '=')
				break;
			else
				size++;
	}
	if(ep == NULL || __builtin_expect(*ep == NULL, 1)) {
		char **new_environ;
		new_environ = (char **) realloc(last_environ, (size + 2) * sizeof(char *));

		if(new_environ == NULL)
			return -1;

		if(combined != NULL) 
			new_environ[size] = (char *) combined;
		else {
			new_environ[size] = (char *) malloc(namelen + 1 + vallen);
			if(__builtin_expect(new_environ[size] == NULL, 0))
				return - 1;
			memcpy(new_environ[size], name, namelen);
			new_environ[size][namelen] = '=';
			memcpy(&new_environ[size][namelen + 1], value, vallen);
		}
		if(environ != last_environ)
			memcpy((char *) new_environ, (char *)environ, size * sizeof(char *));
		new_environ[size + 1] = NULL;
		last_environ = environ = new_environ;
	} else if(replace) {
		char *np;

		if(combined != NULL)
			np = (char *) combined;
		else {
			np = malloc (namelen + 1 + vallen);
			if (__builtin_expect (np == NULL, 0))
				return -1;
			memcpy (np, name, namelen);
			np[namelen] = '=';
			memcpy (&np[namelen + 1], value, vallen);
		}
		*ep = np;
	}
	return 0;
}
