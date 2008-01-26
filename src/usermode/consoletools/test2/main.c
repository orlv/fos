#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <fos/fos.h>
#include <stdlib.h>
#include <fos/message.h>
#include <sched.h>
/*
THREAD(thread) {
	printf("me working.\n");
	while(1) sched_yield();
}
void thread_test() {
	printf("Threading test\n");
	for(int i = 0; i < 256; i++) {
		printf("  Creating thread %u\n", i);
		pid_t child = thread_create((off_t) thread);
		printf("   TID: %x\n", child);
	}
	printf("Passed!\n");
}
*/
THREAD(thread) {
	struct message msg;
	while(1) {
	        msg.tid = 0;
		msg.recv_size = 0;
		msg.flags = 0;
		receive(&msg);
		msg.arg[2] = msg.arg[0] * msg.arg[1];
		msg.send_size = 0;
		reply(&msg);
	}
}
void message_test() {
	printf("Message test\n");
	pid_t child = thread_create((off_t) thread);
	printf(" Created server: %u\n", child);
	struct message msg;
	msg.tid = child;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	int passed = 0;
	int failed = 0;
	for(int i = 0; i < 100; i++) {
	for(int j = 0; j < 100; j++) {
		msg.arg[0] = i;
		msg.arg[1] = j;
		send(&msg);
		int valid = i * j;
		if(msg.arg[2] == valid)
			passed++;
		else
			failed++;
		printf("%s: check %d * %d: must be %d, server returns %d\n", msg.arg[2] == valid ? "passed" : "FAILED!", i, j, valid, msg.arg[2]);
	}
	printf("%u checks passed, %u checks failed\n", passed, failed);
	}
	while(1) sched_yield();
}
asmlinkage int main(int argc, char **argv)
{	// thread_test();
	message_test();

	return 0;
}
