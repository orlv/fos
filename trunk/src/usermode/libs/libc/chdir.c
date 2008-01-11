#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int chdir(const char *path) {
	char *oldpwd = getenv("OLDPWD");
	char *pwd = getenv("PWD");
	char *fullpath = (char *)path;
	if(path[0] != '/') {
		fullpath = malloc(strlen(pwd) + strlen(path) + 1);
		strcpy(fullpath, pwd);
		strcat(fullpath, path);
	}
	// TODO: делать тру-путь
	DIR *dir = opendir(fullpath);
	if(!dir) {
		if(fullpath != path) free(fullpath);
		return -1;
	}
	closedir(dir);
	if(!oldpwd) {
		printf("First chdir\n");
		char *data = malloc(strlen(pwd) + 8);
		strcpy(data, "OLDPWD=");
		strcat(data, pwd);
		putenv(data);
	}else {
		free(getenv("OLDPWD") - 7);
		char *data = malloc(strlen(pwd) + 8);
		strcpy(data, "OLDPWD=");
		strcat(data, pwd);
		putenv(data);
		free(getenv("PWD") - 4);
	}
	char *data = malloc(strlen(fullpath) + 5 + 1);
	strcpy(data, "PWD=");
	strcat(data, fullpath);
	strcat(data, "/");
	putenv(data);
	if(fullpath != path) free(fullpath);
	return 0;
}
