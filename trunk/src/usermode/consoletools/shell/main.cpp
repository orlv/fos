/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fos/fos.h>
#include <unistd.h>
#include <fcntl.h>

#include "readline.h"

void cd_buitin(char *directory);
void pwd_builtin(char *reserved);
void exit_builtin(char *reserved);
void set_builtin(char *arg);
void unset_builtin(char *arg);
void help_builtin(char *arg);
void echo_builtin(char *arg);

static void exec_script(char *filename);

static const struct {
	char *name;
	void (*builtin)(char *);
} buitins[] = {
	{"cd", cd_buitin },
	{"pwd", pwd_builtin },
	{"exit", exit_builtin },
	{"logout", exit_builtin },
	{"export", set_builtin },
	{"set", set_builtin },
	{"unset", unset_builtin },
	{"help", help_builtin },
	{"echo", echo_builtin },
	{"source", exec_script },
	{".", exec_script  },
	{ NULL, NULL },
};

void echo_builtin(char *arg) {
	printf("%s\n", arg);
}

void help_builtin(char *arg) {
	printf("List of builtin commands:\n");
	for(int i = 0; buitins[i].name; i++) 
		printf("%s\n", buitins[i].name);
}

void cd_buitin(char *directory) {
	if(chdir(directory) < 0)
		printf("sh: can't change directory\n");

}

void unset_builtin(char *arg) {
	unsetenv(arg);
}

void set_builtin(char *arg) {
	if(!strlen(arg)) {
		for(int i = 0; environ[i]; i++)
			printf("%s\n", environ[i]);
	} else {
		if(!strchr(arg, '='))
			return;
		else {
			char *buf = new char[strlen(arg) + 1];
			strcpy(buf, arg);
			if(putenv(buf) < 0)
				delete buf;
		}
	}
}

void pwd_builtin(char *reserved) {
	printf("%s\n", getenv("PWD"));
}

void exit_builtin(char *reserved) {
	printf("exit\n");
	exit(0);
}

static int ExecFromPATH(char *cmd, char *args) {
	int pid = 0;
	if(cmd[0] == '/') {	// абсолютный путь
		return exec(cmd, args);
	} else if(strchr(cmd, '/')) { 	// относительный путь
		char *pwd = getenv("PWD");
		char *buf = (char *)malloc(strlen(cmd) + strlen(pwd) + 1);
		strcpy(buf, pwd); strcat(buf, cmd);
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
	while(cmd[0] == ' ') cmd++;
	if(cmd[0] == 0 || cmd[0] == '#') return;
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

static void interactive_shell() {
	exec_script("/root/.login");
	char *cmd = new char[256];
	readline_context *rc = readline_init(16);
	if(!rc) {
		printf("Readline failure, aborting.\n");
		exit(1);
	}

	while(1) {
		char *pwd = getenv("PWD");
		char *ps = getenv("PS1");
		if(ps)
			printf(ps, pwd);
		else
			printf("$ ");
		fflush(stdout);
		readline(cmd, 256, rc);
//		cmd[strlen(cmd) - 1] = 0; // убиваем перевод строки
		char *args = strchr(cmd, 0x20);
		if(args) {
			args++;
			cmd[args - cmd - 1] = 0;
		} else 
			args = "";
		eval(cmd, args);
	}
}

static void exec_script(char *filename) {
	int hndl = open(filename, 0);
	int size = lseek(hndl, 0, SEEK_END);
	lseek(hndl, 0, SEEK_SET);
	char *script = new char[size];
	char *scr = script;
	read(hndl, script, size);
	close(hndl);
	for (char *ptr = strsep(&scr, "\n"); ptr; ptr = strsep(&scr, "\n")) {
		char *args = strchr(ptr, 0x20);
		if(args) {
			args++;
			ptr[args - ptr - 1] = 0;
		} else 
			args = "";
		eval(ptr, args);
		
	}
	free(scr);
	return;
}

int main(int argc, char *argv[]) {
	if(argc < 2)
		interactive_shell();
	else
		exec_script(argv[1]);
	return 0;

}
