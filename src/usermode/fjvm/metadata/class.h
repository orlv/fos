#ifndef CLASS_H_
#define CLASS_H_

#include "constpool/constpool.h"

#include "../types.h"
#include "../synch.h"
#include "../hash.h"
#include "../types.h"
#include "../jar.h"
#include "../jit/objbuf.h"

#define CLASS_MAGIC     0xCAFEBABE

#define ACC_PUBLIC          0x0001  /* Class, Method, Variable  */
#define ACC_PRIVATE         0x0002  /* Method, Variable         */
#define ACC_PROTECTED       0x0004  /* Method, Variable         */
#define ACC_STATIC          0x0008  /* Method, Variable         */
#define ACC_FINAL           0x0010  /* Class, Method, Variable  */
#define ACC_SYNCHRONIZED    0x0020  /* Method                   */
#define ACC_VOLATILE        0x0040  /* Variable (Can't cache)   */
#define ACC_TRANSIENT       0x0080  /* Variable (Not to be written or
                                        read by a persistent object manager) */
#define ACC_NATIVE          0x0100  /* Method                   */
#define ACC_INTERFACE       0x0200  /* Class                    */
#define ACC_ABSTRACT        0x0400  /* Class, Method            */

#define CLASS_LOADING       1   /* Class is in loading state */
#define CLASS_LOADED        2   /* Class is loaded but not linked yet */
#define CLASS_LINKING       3   /* Class is in linking state */
#define CLASS_LINKED        4   /* Class is linked and ready to use */
#define CLASS_BAD           5   /* Class is not loaded or not linked properly and should be removed */

struct Class_s;

typedef
struct ExceptionInfo_s {
	u2		start_pc;
	u2		end_pc;
	u2		handler_pc;
	u2		catch_type;
} ExceptionInfo_t;
                                    
typedef
struct FieldInfo_s {
	u2			access_flags;
	char	   *name;
	char       *signature;
	
	void	   *static_value;
	u2			constant;
	
	int 		offset;
} FieldInfo_t;

typedef void(*native_func_t)();

typedef
struct MethodInfo_s {
	u2			access_flags;
	char	   *name;
	char       *signature;
	
	struct Class_s    *owner;
	
	hash_t 		hash;
	
	int 		index;
	
	int 		code_length;
	u1		   *code;
	
	int			arg_count;
	
	int			max_locals;
	int 		max_stack;
	
	native_func_t native;
	
	u2 		    exception_table_length;
	ExceptionInfo_t *exceptions;
	
	u2			throw_table_length;
	u2		   *throw_table;
	
	ObjBuffer_t *jited;
	
	mutex_t     mutex;
} MethodInfo_t;


                                    
typedef struct Class_s Class_t;
struct Class_s {
	char       *name;
	char	   *super_name;
	
	Class_t    *super;
	struct ConstPool_s *cp;
	
	u2		    access_flags;
	
	u2		    interface_count;
	Class_t	  **interfaces;
	
	u2			field_count;
	FieldInfo_t*fields;
	
	u2			method_count;
	MethodInfo_t*methods;
	
	u1			state;
	
	u2			vmt_count;
	MethodInfo_t**vmt;
	
	size_t		object_size;
};

typedef struct Object_s Object_t;
struct Object_s {
	Class_t     *class;
	u1			*data;
	size_t		data_size;
	u1			arr_type;
	count_t     arr_size;
	
	mutex_t     mutex;
};

#define SCAN_SIG(p, D, S)                               \
   p++;                    \
   while(*p != ')') {                                   \
       if((*p == 'J') || (*p == 'D')) {         \
          D;                                            \
          p++;                                          \
      } else {                                          \
          S;                                            \
          if(*p == '[')                                 \
              for(p++; *p == '['; p++);         \
          if(*p == 'L')                                 \
              while(*p++ != ';');           \
          else                                              \
              p++;                                      \
      }                                                         \
   }                                                            \
   p++;               /* skip end ')' */

char    * class_load(char *name);
Class_t * class_load_embedded(char *name, void *data);
Class_t * class_define(const char *name, void *data);
Class_t * class_resolve(char *name);
void      class_link(Class_t *class);

MethodInfo_t * method_resolve(Class_t *class, hash_t hash);
Class_t      * class_method_resolve(Class_t *class, hash_t hash);
int            method_idx_resolve(Class_t *class, hash_t hash);
FieldInfo_t  * field_resolve(Class_t *class, char *name); 

Object_t * object_new(Class_t *class);
Object_t * object_clone(Object_t *obj);
Object_t * object_new_array(u1 type, count_t length);
Object_t * object_new_string(char *str);
Object_t * object_new_exception(Class_t *class, char *str);

native_func_t native_resolve(hash_t class, hash_t method);

bool_t class_is_super(Class_t *class, Class_t *parent);

static ZipFile_t *classpath;
static int cp_length;
static int cp_inited = 0;

#endif /*CLASS_H_*/
