/*
 * Copyright (c) 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int not_first_chdir = 0;
int chdir(const char *path) {
	char *pwd = getenv("PWD");
	char *fullpath = (char *)path;
	char *real = malloc(1024);
	if(path[0] != '/') {
		fullpath = malloc(strlen(pwd) + strlen(path) + 1);
		strcpy(fullpath, pwd);
		strcat(fullpath, path);
	}
	if(!realpath(fullpath, real)) {
		if(fullpath != path) free(fullpath);
		free(real);
		return -1;
	}
	if(not_first_chdir) 
		free(getenv("PWD") - 4);
	
	not_first_chdir = 1;
	if(!strcmp(real, "/")) real[0] = 0;
	char *data = malloc(strlen(real) + 4 + 1);
	strcpy(data, "PWD=");
	strcat(data, real);
	strcat(data, "/");
	putenv(data);
	if(fullpath != path) free(fullpath);
	free(real);
	return 0;
}
