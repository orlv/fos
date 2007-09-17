#ifndef BASICTYPE_H_
#define BASICTYPE_H_

#include "../types.h"

typedef int BType_t;

#define T_BOOLEAN 4
#define T_CHAR    5
#define T_FLOAT   6
#define T_DOUBLE  7
#define T_BYTE    8
#define T_SHORT   9
#define T_INT     10
#define T_LONG    11
#define T_OBJECT  13

bool_t isWide(BType_t type);
bool_t isPrimitive(BType_t type);
bool_t isArray(BType_t type);
char   getChar(BType_t type);
int    getSize(BType_t type);

#endif /*BASICTYPE_H_*/
