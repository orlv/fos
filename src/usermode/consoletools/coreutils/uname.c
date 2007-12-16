#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fos/fos.h>
int kernelauto = 1;
int kernel = 1;
int nodename = 0;
int release = 0;
int version = 0;
int machine = 0;
int processor = 0;
int platform = 0;
int os = 0;
void pversion();
void usage();
void parse_dmesg(char *buf, char *release, char *version, char *kernel);
int main(int argc, char *argv[]) {
	int opt;
	while((opt = getopt(argc, argv, "asnrvmpiohV")) != -1) {
		switch(opt) {
		case 'a':
			kernelauto = 0;
			kernel = 1;
			nodename = 1;
			release = 1;
			version = 1;
			machine = 1;
			processor = 0;
			platform = 0;
			os = 1;
			break;
		case 's':
			kernelauto = 0;
			kernel = 1;
			break;
		case 'n':
			if(kernelauto) kernel = 0;
			nodename = 1;
			break;
		case 'r':
			if(kernelauto) kernel = 0;
			release = 1;
			break;
		case 'v':
			if(kernelauto) kernel = 0;
			version = 1;
			break;
		case 'm':
			if(kernelauto) kernel = 0;
			machine = 1;
			break;
		case 'p':
			if(kernelauto) kernel = 0;
			processor = 1;
			break;
		case 'i':
			if(kernelauto) kernel = 0;
			platform = 1;
			break;
		case 'o':
			if(kernelauto) kernel = 0;
			os = 1;
			break;
		case 'h':
			usage();
			exit(0);
			break;
		case 'V':
			pversion();
			exit(0);
			break;
		default:
			usage();
			exit(1);
			break;
		}
	}
	char *dmsg = NULL, *drelease = NULL, *dversion = NULL, *dkernel = NULL;
	if(release || version || kernel) {
		dmsg = malloc(1024);
		dmesg(dmsg, 1024);
		drelease = malloc(32);
		dversion = malloc(32);
		dkernel = malloc(32);
		parse_dmesg(dmsg, drelease, dversion, dkernel);
	}
	if(kernel)
		printf("%s ", dkernel);
	if(nodename)
		printf("localhost ");
	if(release)
		printf("%s ", drelease);
	if(version)
		printf("%s ", dversion);
	if(machine)
		printf("i386 ");
	if(processor)
		printf("unknown ");
	if(platform)
		printf("unknown ");
	if(os) 
		printf("FOS ");
	printf("\n");
	if(dmsg) {
		free(dmsg);
		free(drelease);
		free(dversion);
	}
	return 0;
}

void usage() {
	printf("Usage: uname [OPTION]...\n"
#ifndef	NOT_INCLUDE_HELP
	"Print certain system information.  With no OPTION, same as -s.\n\n"
	"  -a    print all information, in the following order,\n"
	"          except omit -p and -i if unknown:\n"
	"  -s    print the kernel name\n"
	"  -n    print the network node hostname\n"
	"  -r    print the kernel release\n"
	"  -v    print the kernel version\n"
	"  -m    print the machine hardware name\n"
	"  -p    print the processor type or \"unknown\"\n"
	"  -i    print the hardware platform or \"unknown\"\n"
	"  -o    print the operating system\n"
	"  -h    display this help and exit\n"
	"  -V    output version information and exit\n\n"
	"Report bugs to <grindars@grindars.org.ru>\n"
#endif
	);
}
void pversion() {
	printf("uname (FOS tiny coreutils) 0.1\n");
}

void parse_dmesg(char *buf, char *release, char *version, char *kernel) {
	char *firstline = strsep(&buf, "\n");
	char *tokens[16];
	int i = 0;
	for(char *ptr= strsep(&firstline, " "); ptr && i < 16; ptr= strsep(&firstline, " "), i++) 
		tokens[i] = ptr;
	
	strcpy(kernel, tokens[1]);
	strcpy(version, tokens[2]);
	tokens[3][strlen(tokens[3]) - 1] = 0;
	strcpy(release, tokens[3] + 1);
	
}
