#include "synch.h"
#include "xchg.h"
#include <stdlib.h>

void synch_init(mutex_t *mutex) {
    mutex->flock = 0;
}

void synch_delete(mutex_t *mutex) {
    free(mutex);
}

void synch_lock(mutex_t *mutex) {
	return;
	
	while (mutex->flock);
		
    int result;
    
    do {
		result = xchg(&mutex->flock, 1);
    } while (result != 0);
}

//bool_t synch_try_lock(mutex_t *mutex) { return xchg(&mutex->flock, 1) == 0; }

void synch_unlock(mutex_t *mutex) {
    xchg(&mutex->flock, 0);
}
