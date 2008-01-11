/*
  Copyright (C) 2008 Sergey Gridassov
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fos/fos.h>
#include <unistd.h>
void cd_buitin(char *directory);
static const struct {
	char *name;
	void (*builtin)(char *);
} buitins[] = {
	{"cd", cd_buitin },
	{ NULL, NULL },
};

void cd_buitin(char *directory) {
	if(chdir(directory) < 0)
		printf("sh: can't change directory\n");

}

static int ExecFromPATH(char *cmd, char *args) {
	int pid = 0;
	if(cmd[0] == '/') {	// абсолютный путь
		return exec(cmd, args);
	} else if(strchr(cmd, '/')) { 	// относительный путь
		char *pwd = getenv("PWD");
		char *buf = (char *)malloc(strlen(cmd) + strlen(pwd) + 1);
		strcpy(buf, pwd); strcat(buf, cmd);
		printf("Relative path, trying %s\n", buf);
		pid = exec(buf, args);
		free(buf);
	} else {			// поиск через PATH.
		char *path = getenv("PATH");
		char *buf = (char *)malloc(strlen(path) + strlen(cmd) + 2); // это не бред, как может показатся.
		for(char *ptr = path; ptr; ptr = strchr(ptr, ':')) {
			if(ptr[0] == ':') ptr++;
			char *next = strchr(ptr, ':');
			if(next)
				strncpy(buf, ptr, next - ptr);
			else
				strcpy(buf, ptr);
			strcat(buf, "/");
			strcat(buf, cmd);
			pid = exec(buf, args);
			if(pid) break;
		}
		free(buf);
	}
	return pid;
}
static void eval(char *cmd, char *args) {
	for(int i = 0; buitins[i].name; i++) {
		if(!strcmp(cmd, buitins[i].name)) {
			(buitins[i].builtin)(args);
			return;
		}
	}
	int pid = ExecFromPATH(cmd, args);
	if(!pid)
		printf("sh: %s: command not found\n", cmd);
}
int main(int argc, char *argv[]) {
	printf("Welcome to FOS Operating System\n");
	char *cmd = new char[256];
	while(1) {
		char *pwd = getenv("PWD");
		printf("%s # ", pwd);
		fgets(cmd, 256, stdin);
		cmd[strlen(cmd) - 1] = 0; // убиваем перевод строки
		char *args = strchr(cmd, 0x20);
		if(args) {
			args++;
			cmd[args - cmd - 1] = 0;
		} else 
			args = "";
		eval(cmd, args);
	}
}
