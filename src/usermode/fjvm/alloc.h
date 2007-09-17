#ifndef ALLOC_H_
#define ALLOC_H_

void *jvm_malloc(int size);
void  jvm_free(void *ptr);

#endif /*ALLOC_H_*/
