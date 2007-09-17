#ifndef EXEC_H_
#define EXEC_H_

#include "metadata/class.h"
#include "types.h"
#include "hash.h"
#include "heap.h"

typedef struct Frame_s Frame_t;
struct Frame_s {
	u1		*cp;
	int		*locals;
	
	Frame_t *prev;
	Class_t *class;
	
	MethodInfo_t *method;
};

typedef struct Exec_s Exec_t;
struct Exec_s {
	hash_table_t			*strings;
	hash_table_t			*classes;
	
	Heap_t					*heap;
	
	int						*stack;
	int						*sp;
	
	Object_t				*exception;
	
	Frame_t					*frame;
};

#define POP_INT()       ((int)*(--(exec->sp)))
#define POP_FLOAT()     (*((float*)(--(exec->sp))))
#define POP_LONG()      ((long long int)*(--(exec->sp))) | (((long long int)*(--(exec->sp))) << 32)
#define POP_OBJECT()    ((Object_t *)*(--(exec->sp)))

#define PUSH_INT(v)     *(exec->sp++)=(int)(v)
#define PUSH_FLOAT(v)   *((float*)(exec->sp++))=(v)
#define PUSH_LONG(v)    *(exec->sp++)=(int)((v)>>32); *(exec->sp++)=(int)(v)
#define PUSH_OBJECT(v)  *(exec->sp++)=(int)(v)

#define HASH_SIZE_UTF8      128
#define HASH_SIZE_CLASS     128

Exec_t *exec_create(size_t heapSize);
Exec_t *exec_get_current();
void    exec_delete();

void    exec_rise_exception(Object_t *exception);

Frame_t *frame_create(Class_t *class, MethodInfo_t *method);
void     frame_delete();


#endif /*EXEC_H_*/
