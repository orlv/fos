/* (C) 2008 Serge Gridassov */

#include <sys/socket.h>
#include <sys/un.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
	int s = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un local;
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, "./pipe");
	unlink(local.sun_path);
	bind(s, (struct sockaddr *)&local, sizeof(local));

	listen(s, 5);

	char *buf = malloc(256);

	while(1) {
		printf("*** Waiting for connection\n");
		size_t len = sizeof(struct sockaddr_un);
		struct sockaddr_un remote;
		int s2 = accept(s, (struct sockaddr *)&remote, &len);
		if(s2 == -1) break;
		printf("*** Streaming data\n");
		while(1) {
			len = recv(s2, buf, 256, 0);
			if(len < 1) break;
			fwrite(buf, len, 1, stdout);
			fflush(stdout);
		}
		printf("*** Connection loss\n");
	}
	return 0;

}
