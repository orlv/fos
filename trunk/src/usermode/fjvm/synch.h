#ifndef SYNCH_H_
#define SYNCH_H_

typedef
struct mutex_s {
    int flock;
} mutex_t;

void synch_init(mutex_t *mutex);
void synch_delete(mutex_t *mutex);
void synch_lock(mutex_t *mutex);

//bool_t synch_try_lock(mutex_t *mutex);

void synch_unlock(mutex_t *mutex);

#endif /*SYNCH_H_*/
