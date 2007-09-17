#ifndef _CONSTPOOL_H
#define _CONSTPOOL_H

#include "constobject.h"

typedef
struct ConstPool_s {
	int 		    count;
	struct ConstObject_s **pool;
} ConstPool_t;

ConstPool_t * constpool_create(int count);
int           constpool_length(ConstPool_t *pool);
void          constpool_delete(ConstPool_t *pool);

double        constpool_get_double(ConstPool_t *pool, int index);
long long     constpool_get_long(ConstPool_t *pool, int index);
float		  constpool_get_float(ConstPool_t *pool, int index);
int           constpool_get_integer(ConstPool_t *pool, int index);

struct ConstSign_s *constpool_get_signature(ConstPool_t *pool, int index);
struct ConstClass_s *constpool_get_class(ConstPool_t *pool, int index);

char *        constpool_get_string(ConstPool_t *pool, int index);

struct ConstMethodref_s  * constpool_get_method(ConstPool_t *pool, int index);
struct ConstIMethodref_s * constpool_get_imethod(ConstPool_t *pool, int index);
struct ConstFieldref_s   * constpool_get_field(ConstPool_t *pool, int index);

ConstPool_t *      constpool_read(unsigned char * ptr, int *readed);

#endif /*_CONSTPOOL_H*/
