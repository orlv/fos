#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
void usage();
void cat(char *file);
void version();

int main(int argc, char *argv[]) {
	int opt;
	while((opt = getopt(argc, argv, "hV")) != -1) {
		switch(opt) {
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
		cat("-");
		return 0;
	}
	for(int i = optind; i < argc; i++) {
		cat(argv[i]);
	}
	return 0;
}
void usage() {
	printf("Usage: cat [OPTION] [FILE]...\n"
#ifndef NOT_INCLUDE_HELP
	"Concatenate FILE(s), or standard input, to standard output.\n"
        "  -h               display this help and exit\n"
	"  -V               output version information and exit\n\n"
	"With no FILE, or when FILE is -, read standard input.\n\n"
	"Examples:\n"
	"  cat f - g  Output f's contents, then standard input, then g's contents.\n"
	"  cat        Copy standard input to standard output.\n\n"
	"Report bugs to <grindars@grindars.org.ru>.\n"
#endif
	);
}
void cat(char *file) {
	// TODO: сделать чтение stdin.
	if(!strcmp(file, "-")) {
		printf("cat: reading standard input not implemented\n");
		return;
	}
	int hndl = open(file, 0);
	if(hndl == -1) {
		printf("cat: %s: No such file or directory\n", file);
		return;
	}
	struct stat st;
	fstat(hndl, &st);
	char *buf = malloc(st.st_size);
	read(hndl, buf, st.st_size);
	// TODO: когда будут потоки, сделать нормально.
	// смысл этого кода в том, что бы записать не до нуля, а
	// до конца файла. Будет что-то вроде write(stdout, buf, st.st_size);
	int tty = open("/dev/tty", 0);
	write(tty, buf,  st.st_size);
	close(tty);
	free(buf);	
	close(hndl);
}
void version() {
	printf("cat (FOS tiny coreutils) 0.1\n");
}
