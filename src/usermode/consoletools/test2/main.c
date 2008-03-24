#include <fos/fos.h>
#include <stdio.h>
#include <sched.h>
#include <mutex.h>

mutex_t mutex = 0;

void thread_b() {
	printf("THREAD B: locking mutex\n");
	while(!mutex_try_lock(&mutex)) sched_yield();
	printf("THREAD B: locked (deadlock)\n");
	while(1) sched_yield();
}

asmlinkage int main(int argc, char **argv)
{
	thread_create((off_t)thread_b, 0);
	printf("THREAD A: sleeping\n");
	for(int i = 0; i < 800; i++) sched_yield();
	printf("THREAD A: locking mutex\n");
	while(!mutex_try_lock(&mutex)) sched_yield();
	printf("THREAD A: locked, mutex not working correctly!\n");	
	return 0;
}
