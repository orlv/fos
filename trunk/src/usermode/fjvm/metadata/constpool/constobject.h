#ifndef CONSTOBJECT_H_
#define CONSTOBJECT_H_

#define CONST_CLASS       7
#define CONST_FIELDREF    9
#define CONST_METHODREF   10
#define CONST_IMETHODREF  11
#define CONST_STRING      8
#define CONST_INTEGER     3
#define CONST_FLOAT       4
#define CONST_LONG        5
#define CONST_DOUBLE      6
#define CONST_NAMEANDTYPE 12
#define CONST_UTF8        1

#include "../class.h"

typedef
struct ConstObject_s {
	unsigned char   tag;
	void           *data;
} ConstObject_t;

typedef
struct ConstClass_s {
	int nameID;
} ConstClass_t;

typedef
struct ConstDouble_s {
	double value;
} ConstDouble_t;

typedef
struct ConstMethodref_s {
	int classID;
	int signID;
	
	int index;
	int arg_count;
} ConstMethodref_t;

typedef
struct ConstFieldref_s {
	int classID;
	int signID;
	
	struct FieldInfo_s *field;
} ConstFieldref_t;

typedef
struct ConstIMethodref_s {
	int classID;
	int signID;
} ConstIMethodref_t;

typedef
struct ConstInteger_s {
	int value;
} ConstInteger_t;

typedef
struct ConstLong_s {
	long long value;
} ConstLong_t;

typedef
struct ConstSign_s {
	int nameID;
	int descID;
} ConstSign_t;

typedef
struct ConstSingle_s {
	float value;
} ConstSingle_t;

typedef
struct ConstString_s {
	int stringID;
} ConstString_t;

typedef
struct ConstUTF8_s {
	int  length;
	char *data;
} ConstUTF8_t;

ConstObject_t   *constobject_create(unsigned char tag, void *data);
ConstInteger_t  *constobject_integer(int i);
ConstSingle_t   *constobject_float(int i);
ConstDouble_t   *constobject_double(long long i);
ConstLong_t     *constobject_long(long long i);
ConstClass_t    *constobject_class(int i);
ConstString_t   *constobject_string(int i);

ConstFieldref_t *constobject_field(int i, int j);
ConstMethodref_t*constobject_method(int i, int j);
ConstIMethodref_t*constobject_imethod(int i, int j);

ConstSign_t * constobject_sign(int i, int j);

ConstUTF8_t * constobject_str(int i, char *d);

#endif /*CONSTOBJECT_H_*/
