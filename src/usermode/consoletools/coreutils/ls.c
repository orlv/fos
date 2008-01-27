/*
 * Copyright (c) 2007 - 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int many = 0;	 // перед содержимым выводить префикс
int hide_dot = 1;// скрывать файлы c точки
int hide_up_self = 0; // скрывать хардлинки .. и .
int long_format = 0; // полный листинг

void version() {
	printf("ls (FOS tiny coreutils) 0.1\n");
}

void format_permissions(char *buf, mode_t mode) {
	buf[0] = '-';
	buf[1] = (mode & S_IRUSR) ? 'r' : '-';
	buf[2] = (mode & S_IWUSR) ? 'w' : '-';
	buf[3] = (mode & S_IXUSR) ? 'x' : '-';

	buf[4] = (mode & S_IRGRP) ? 'r' : '-';
	buf[5] = (mode & S_IWGRP) ? 'w' : '-';
	buf[6] = (mode & S_IXGRP) ? 'x' : '-';

	buf[7] = (mode & S_IROTH) ? 'r' : '-';
	buf[8] = (mode & S_IWOTH) ? 'w' : '-';
	buf[9] = (mode & S_IXOTH) ? 'x' : '-';

	buf[10] = 0;
}

void ls(char *dir) {
	DIR *dr = opendir(dir);
	if(!dr) {
		printf("ls: cannot access %s\n", dir);
		return;
	}
	char *name = malloc(1024);
	if(many)
		printf("%s:\n", dir);
	for(struct dirent *ptr = readdir(dr); ptr; ptr = readdir(dr)) {
		if(ptr->d_name[0] == '.' && hide_dot) continue;
		if(hide_up_self && (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, ".."))) continue;
		if(long_format) {
			struct stat st;
			strcpy(name, dir);
			if(dir[strlen(dir) - 1] != '/') 
				strcat(name, "/");
			strcat(name, ptr->d_name);
			char buf[11];
			if(stat(name, &st) == -1)
				printf("---------- 1 root root %8u %s\n", 0, ptr->d_name);
			else {
				format_permissions((char *)&buf, st.st_mode);
				printf("%s 1 root root %8u %s\n", &buf, st.st_size, ptr->d_name);
			}
		} else
			printf("%s\n", ptr->d_name);
	}
	closedir(dr);
}

void usage() {
	printf("Usage: ls [OPTION]... [FILE]\n"
#ifndef NOT_INCLUDE_HELP
	"List information about the FILEs (the current directory by default)\n\n"
	"  -a, --all        do not ignore entries starting with .\n"
	"  -A, --almost-all do not list implied . and ..\n"
	"  -l               use a long listing format\n"
        "  -h               display this help and exit\n"
	"  -V               output version information and exit\n\n"
	"Report bugs to <grindars@grindars.org.ru>.\n"
#endif
	);
}


userlinkage int main(int argc, char *argv[]) {
	int opt;
	while((opt = getopt(argc, argv, "aAlhV")) != -1) {
		switch(opt) {
		case 'a':
			hide_dot = 0;
			break;
		case 'A':
			hide_up_self = 1;
			break;
		case 'l':
			long_format = 1;
			break;
		case 'h':
			usage();
			exit(0);
		case 'V':
			version();
			exit(0);	
		default:
			usage();
			exit(1);
		}
	}
	if(optind >= argc) {
		ls(".");
		return 0;
	}
	if(argc - optind > 1)
		many = 1;
	for(int i = optind; i < argc; i++) {
		ls(argv[i]);
	}
	return 0;

}
