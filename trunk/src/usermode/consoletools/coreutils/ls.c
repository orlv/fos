#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
int many = 0;	 // перед содержимым выводить префикс
int hide_dot = 1;// скрывать файлы
int hide_up_self = 0; // скрывать хардлинки .. и .
int long_format = 0; // полный листинг
void usage();
void ls(char *dir);
void version();

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
		ls("/");
		return 0;
	}
	if(argc - optind > 1)
		many = 1;
	for(int i = optind; i < argc; i++) {
		ls(argv[i]);
	}
	return 0;

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
			stat(name, &st);
			printf("---------- 1 root root %8u %s\n", st.st_size, ptr->d_name);
		} else
			printf("%s\n", ptr->d_name);
	}
	closedir(dr);
}
void version() {
	printf("ls (FOS tiny coreutils) 0.1\n");
}
