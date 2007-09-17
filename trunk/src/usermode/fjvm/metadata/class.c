#include <stdio.h>

#define FOS 1
#define JAR 1

#include "../alloc.h"
#include "../exec.h"
#include "../heap.h"
#include "../gc.h"

#include "../method.h"

#include "class.h"
#include "basictype.h"

#include "../native.h"

#include "../jar.h"

#include <string.h>
#include <stdio.h>

#if FOS
#include <sys/stat.h>
#include <fos/fos.h>
#endif

Class_t * class_define(const char *name, void *data) {
	unsigned char *ptr = (unsigned char *)data;
	
	int index, i, j, c;
	
	#if VERBOSE
	printf("Defining: %s\n", name);
	#endif
	
	//Exec_t *exec = exec_get_current();
	
	int attr_count, attr_length, attr;
	char *attr_name;
	
	u4 magic;
	READ_U4(magic, ptr);
	if (CLASS_MAGIC != magic) {
		exec_rise_exception(object_new_exception(class_resolve("java/lang/Exception"), "Bad magic"));
		return NULL;
	}
	
	u2 minor, major;
	READ_U2(minor, ptr);
	READ_U2(major, ptr);
	
	int readed = 0;
	ConstPool_t *cp = constpool_read(ptr, &readed);
	ptr += readed;
	
	Class_t *class = (Class_t*)jvm_malloc(sizeof(Class_t));
	class->state = CLASS_LOADING;	
	class->cp = cp;
	
	READ_U2(class->access_flags, ptr);
	
	READ_U2(index, ptr);
	
	index = ((ConstClass_t*)cp->pool[index]->data)->nameID;
	class->name = constpool_get_string(cp, index);
	
	if (name && (strcmp(name, class->name) != 0)) {
		printf("NoClassDefError: %s %s\n", name, class->name);
		exec_rise_exception(object_new_exception(class_resolve("java/lang/Exception"), "NoClassDefError"));
		return NULL;
	}
	
	READ_U2(index, ptr);
	if (strcmp(class->name, "java/lang/Object") == 0) {
		class->super = NULL;
	} else {
		index = ((ConstClass_t*)cp->pool[index]->data)->nameID;
		
		class->super_name = constpool_get_string(cp, index);
		class->super = class_resolve(class->super_name);
	}
	
	READ_U2(class->interface_count, ptr);
	class->interfaces = (Class_t**)jvm_malloc(sizeof(Class_t) * class->interface_count);
	for (i = 0; i < class->interface_count; i++) {
		char *interface_name;
		
		READ_U2(index, ptr);
		index = ((ConstClass_t*)cp->pool[index]->data)->nameID;
		interface_name = constpool_get_string(cp, index);
		class->interfaces[i] = class_resolve(interface_name);
		
		if (NULL == class->interfaces[i]) {
			exec_rise_exception(object_new_exception(class_resolve("java/lang/Exception"), "Interface not found."));
			return NULL;
		}
	}
	
	READ_U2(class->field_count, ptr);
	class->fields = (FieldInfo_t*)jvm_malloc(sizeof(FieldInfo_t) * class->field_count);
	for (i = 0; i < class->field_count; i++) {
		READ_U2(class->fields[i].access_flags, ptr);
		
		READ_U2(index, ptr);
		class->fields[i].name = constpool_get_string(cp, index);
		
		READ_U2(index, ptr);
		class->fields[i].signature = constpool_get_string(cp, index);
		
		READ_U2(attr_count, ptr);
		for (attr = 0; attr < attr_count; attr++) {
			READ_U2(index, ptr);
			READ_U4(attr_length, ptr);
			
			if ( (cp->pool[index]->tag == CONST_UTF8) && (strcmp(constpool_get_string(cp, index), "ConstantValue"))) {
				 READ_U2(class->fields[i].constant, ptr);
			} else {
				class->fields[i].constant = 0;
				ptr += attr_length;
			}
		}
	}
	
	READ_U2(class->method_count, ptr);
	class->methods = (MethodInfo_t*)jvm_malloc(sizeof(MethodInfo_t) * class->method_count);
	for (i = 0; i < class->method_count; i++) {
		READ_U2(class->methods[i].access_flags, ptr);
		READ_U2(index, ptr);
		
		class->methods[i].name = constpool_get_string(cp, index);
		
		READ_U2(index, ptr);
		class->methods[i].signature = constpool_get_string(cp, index);
		class->methods[i].index = -1;
		
		READ_U2(attr_count, ptr);
		for (attr = 0; attr < attr_count; attr++) {
			READ_U2(index, ptr);
			READ_U4(attr_length, ptr);
			
			attr_name = constpool_get_string(cp, index);
			
			if (strcmp(attr_name, "Code") == 0) {
				READ_U2(class->methods[i].max_stack, ptr);
				READ_U2(class->methods[i].max_locals, ptr);
				READ_U4(class->methods[i].code_length, ptr);
				
				class->methods[i].code = (u1*)ptr;
				ptr += class->methods[i].code_length;
				
				READ_U2(class->methods[i].exception_table_length, ptr);
				class->methods[i].exceptions = jvm_malloc(sizeof(ExceptionInfo_t) * class->methods[i].exception_table_length);
				for (c = 0; c < class->methods[i].exception_table_length; c++) {
					READ_U2(class->methods[i].exceptions[c].start_pc, ptr);
					READ_U2(class->methods[i].exceptions[c].end_pc, ptr);
					READ_U2(class->methods[i].exceptions[c].handler_pc, ptr);
					READ_U2(class->methods[i].exceptions[c].catch_type, ptr);
				}
				
				int sub_attr_count;
				u4 sub_attr_length;
				READ_U2(sub_attr_count, ptr);
				for (j = 0; j < sub_attr_count; j++) {
					READ_U2(index, ptr);
					READ_U4(sub_attr_length, ptr);
					ptr += sub_attr_length; 
				}
			} else if (strcmp(attr_name, "Exceptions") == 0) {
				READ_U2(class->methods[i].throw_table_length, ptr);
				class->methods[i].throw_table = jvm_malloc(2 * class->methods[i].throw_table_length);
				
				for (j = 0; j < class->methods[i].throw_table_length; j++) {
					READ_U2(class->methods[i].throw_table[j], ptr);
				}
			} else {
				ptr += attr_length;
			}
		}
	}
	
	class->state = CLASS_LOADED;
	return class;
}

void class_link(Class_t *class) {
	#if VERBOSE
	printf("Linking: %s\n", class->name);
	#endif
	
	int field_offset = 0;
	int field_size;
	
	FieldInfo_t *fi;
	
	int i, j;
	
	int new_methods_count = 0;
	
	//Exec_t *exec = exec_get_current();
	
	if (NULL == class) return;
	if (CLASS_LINKED == class->state) {
		#if VERBOSE
		printf("Class already linked.\n");
		#endif
		return;
	}
	if (CLASS_LOADED != class->state) return;
	
	class->state = CLASS_LINKING;
	
	Class_t *super = (class->access_flags & ACC_INTERFACE) ? NULL : class->super;
	if (super) {
		if (class->state != CLASS_LINKED) {
			#if VERBOSE
			printf("Have not linked super, linking.\n");
			#endif
			class_link(super);
		}
		
		field_offset = super->object_size;
	}
	
	for (fi = class->fields, i = 0; i < class->field_count; i++) {
		if (fi->access_flags & ACC_STATIC) {
			fi->static_value = 0;
			if (fi->signature[0] == 'J' || fi->signature[0] == 'D') {
				fi->offset = 0;
			}
		} else {
			fi->offset = field_offset;
			
			switch (fi->signature[0]) {
				case 'B':
				case 'Z':
					field_size = 1;
					break;
					
				case 'S':
				case 'C':
					field_size = 2;
					break;
			
				case 'I':
				case 'L':
				case '[':
				case 'F':
					field_size = 4;
					break;
				
				case 'D':
				case 'J':
					field_size = 8;
					break;
			}
			
			field_offset += field_size;
		}
	}
	
	class->object_size = field_offset;
	
	int super_mt_size = (super) ? super->method_count : 0;
	MethodInfo_t *mi;
	
	for (mi = class->methods, i = 0; i < class->method_count; i++, mi++) {
		int count = 0;
		char *sig = mi->signature;
		
		SCAN_SIG(sig, count+=2, count++);
		if (mi->access_flags & ACC_STATIC) {
			mi->arg_count = count;
		} else {
			mi->arg_count = count + 1;
		}
		
		mi->hash = utf8_hash(mi->name) ^ utf8_hash(mi->signature);
		
		#if VERBOSE
		printf("\tLinking method: %s\n", mi->name);
		#endif
		
		if (mi->access_flags & ACC_NATIVE) {
			#if VERBOSE
			printf("\t\tLinking native...\t");
			#endif
			mi->native = native_resolve(utf8_hash(class->name), mi->hash);
			
			if (!mi->native) {
				printf("\nUnresolved native: %x:%x (%s#%s!%s)\n",
					utf8_hash(class->name),
					mi->hash,
					class->name,
					mi->name,
					mi->signature
				);
			} else {
				#if VERBOSE
				printf("ok\n");
				#endif
			}
		}
		
		for (j = 0; j < super_mt_size; j++) {
			if ( mi->hash == super->vmt[j]->hash ) {
				mi->index = super->vmt[j]->index;
				break;
			}
		}
		
		if (j == super_mt_size) {
			mi->index = super_mt_size + new_methods_count++;
		}
		
		synch_init(&(mi->mutex));
	}
	
	class->vmt = NULL;
	class->vmt_count = super_mt_size + new_methods_count;
	
	if (!(class->access_flags & ACC_INTERFACE)) {
		class->vmt = (MethodInfo_t**)jvm_malloc(sizeof(MethodInfo_t*) * class->vmt_count);
		
		if (super) {
			memcpy(class->vmt, super->vmt, super_mt_size * sizeof(MethodInfo_t*));
		}
		
		for (j = 0; j < class->method_count; j++) {
			class->vmt[class->methods[j].index] = &(class->methods[j]); 
		}
	}
	
	#if VERBOSE
	printf("Linked\n");
	#endif
	
	class->state = CLASS_LINKED;
}

Class_t * class_load_embedded(char *name, void *data) {
	Class_t *class;
	
	if (!data) return NULL;
	
	class = class_define(name, data);
	if (class) {
		Exec_t *exec = exec_get_current();
		hash_t hash = utf8_hash(class->name);
		hash_entry_t *entry = hash_table_find(exec->classes, hash);
		
		if (entry == NULL) {
			hash_table_add(exec->classes, hash, class);
		}
		
		class_link(class);
		
		MethodInfo_t *clinit = method_resolve(class, utf8_hash("<clinit>") ^ utf8_hash("()V"));
		if (clinit != NULL) {
// TODODODODODO
			// method_invoke(class, NULL, clinit);
		}
	}
	
	return class;
}		

Class_t *class_resolve(char *name) {	
	#if VERBOSE
	printf("Resolving class: %s\n", name);
	#endif
	
	hash_t hash = utf8_hash(name); 
	hash_entry_t *entry = hash_table_find(exec_get_current()->classes, hash);
	
	if (entry) {
		return (Class_t*)entry->data;
	} else {
		#if VERBOSE
		printf("Class not loaded, loading\n");
		#endif
		
		char *data = class_load(name);
		if (data) {
			Class_t *class = class_load_embedded(name, data);
			if (class == NULL) {
				printf("class_resolve: %s cannot be resolved\n", name);
			}
			
			return class;
		} else {
			printf("class_resolve: %s cannot be resolved(FILE)\n", name);
		}
	}
	
	return NULL;
}

Class_t  * class_method_resolve(Class_t *class, hash_t hash) {
	int i;
	
	if (class == NULL) return NULL;
	
	for (i = 0; i < class->method_count; i++) {
		MethodInfo_t *method = &class->methods[i];
		if (method->hash == hash) {
			return class;
		}
	}
	
	if (class->super != NULL) {
		return class_method_resolve(class->super, hash);
	}
	
	return NULL;
}

MethodInfo_t *method_resolve(Class_t *class, hash_t hash) {
	int i;
	
	if (class == NULL) return NULL;
	
	for (i = 0; i < class->method_count; i++) {
		MethodInfo_t *method = &class->methods[i];
		if (method->hash == hash) {
			return method;
		}
	}
	
	if (class->super != NULL) {
		return method_resolve(class->super, hash);
	}
	
	return NULL;
}

int method_idx_resolve(Class_t *class, hash_t hash) {
	int i;
	
	if (!class) {
		return 0;
	}
	
	for (i = 0; i < class->vmt_count; i++) {
		if (class->vmt[i]->hash == hash) {
			return i;
		}
	}
	
	return 0;
}

FieldInfo_t * field_resolve(Class_t *class, char *name) {
	int i;
	for (i = 0; i < class->field_count; i++) {
		FieldInfo_t *field = &class->fields[i];
		
		if (strcmp(name, field->name) == 0)
			return field;
	}
	
	if (class->super != NULL) {
		return field_resolve(class->super, name);
	}
	
	return NULL;
}

bool_t class_is_super(Class_t *class, Class_t *parent) {
	Class_t *cl = class;
	while (cl != NULL) {
		if (cl == parent) {
			return TRUE;
		}
		
		cl = cl->super;
	}
	
	return FALSE;
}

/*
#if FOS && JAR
void init_cp() {

}

#endif
*/


char * class_load(char *name) {
#if !FOS
	char path[1024];
	char *buffer;
	int size;
	
	strcpy(path, "classpath/");
	strcat(path, name);
	strcat(path, ".class");
	
	FILE *file = fopen(path, "rb");
	if (!file) return NULL;
	
	fseek(file, 0, SEEK_END);	
	size = ftell(file);
	
	buffer = malloc(size * sizeof(char));
	
	rewind(file);
	
	fread(buffer, 1, size, file); 
		
	return buffer;
	
#else
#if !JAR
	struct stat *buf = jvm_malloc(sizeof(struct stat));
	char *image;
	int size;
	
	char path[1024];
	
	strcpy(path, "/mnt/modules/classpath/");
	strcat(path, name);
	strcat(path, ".class");
	
	int fd = open(path, 0);
	fstat(fd, buf);
	
	image = jvm_malloc(buf->st_size);
	read(fd, image, buf->st_size);
	
	return image;
#else
	struct stat *buf = jvm_malloc(sizeof(struct stat));
	const char *classpath_path = "/mnt/modules/classpath.jar";
	int fd;
	int size;
	unsigned char *bcpjar;
	
	char path[1024];
	char *image;

	if (!cp_inited) {
		fd = open(classpath_path, 0);
		
		#if VERBOSE
		printf("Loading classpath...\n");
		#endif
		
		fstat(fd, buf);
	
		size = buf->st_size;
		cp_length = size;
		bcpjar = jvm_malloc(size * 4);
		
		read(fd, bcpjar, size);
		
		#if VERBOSE
		printf("classpath readed: %i bytes\n", size);
		#endif
	
		classpath = zip_process(bcpjar, 4 * size);
		if (classpath == NULL) {
			printf("Failed to load classpath.\n");
			return NULL;
		}
		cp_inited = 1;
	}

	strcpy(path, "classpath/");
	strcat(path, name);
	strcat(path, ".class");
	
	int len;
	
	#if VERBOSE
	printf("Loading class: %s", path);
	#endif
	
	image = zip_find_entry(path, classpath, &len);
	
	return image;
#endif
#endif
}

Object_t * object_new(Class_t *class) {
	if (!class) return NULL;
	
	Exec_t *exec = exec_get_current();
	Object_t *obj = heap_alloc(exec->heap, sizeof(Object_t), GC_TEST | GC_OBJECT);
	obj->class = class;
	
	obj->data = NULL;
	obj->arr_type = 0;
	
	if (class->object_size) {
		obj->data = heap_alloc(exec->heap, class->object_size, GC_TEST | GC_DATA);
		memset(obj->data, 0, class->object_size);
	}
	
	obj->data_size = class->object_size;
	
	synch_init(&(obj->mutex));
	return obj;
}

Object_t * object_clone(Object_t *obj) {
	if (obj == NULL) return NULL;
	
	Exec_t *exec = exec_get_current();
	
	Object_t *clone = heap_alloc(exec->heap, sizeof(Object_t), GC_TEST | GC_OBJECT);
	clone->class = obj->class;
	if (obj->data_size) {
		clone->data = heap_alloc(exec->heap, obj->class->object_size, GC_TEST | GC_DATA);
		memcpy(clone->data, obj->data, obj->class->object_size);
	}
	
	clone->arr_type = obj->arr_type;
	clone->data_size = obj->data_size;
	clone->arr_size = obj->arr_size;
	
	synch_init(&(clone->mutex));
	return clone;
}

Object_t * object_new_array(u1 type, count_t length) {
	Exec_t *exec = exec_get_current();
	Object_t *obj = object_new(class_resolve("java/lang/Object"));
	
	MethodInfo_t *constructor = method_resolve(obj->class, utf8_hash("<init>") ^ utf8_hash("()V"));
	*(exec->sp++) = (int)obj;
	
	method_invoke(obj->class, obj, constructor);
	
	obj->arr_type = type;
	int type_size = 0;
	
	switch (type) {
		case T_BOOLEAN:
		case T_BYTE:
			type_size = 1;
			break;
			
		case T_CHAR:
		case T_SHORT:
			type_size = 2;
			break;
			
		case T_FLOAT:
		case T_INT:
		case T_OBJECT:
			type_size = 4;
			break;
		
		case T_LONG:
		case T_DOUBLE:
			type_size = 8;
			break;
		
	}
	
	obj->data_size = type_size * length;
	obj->data = heap_alloc(exec->heap, obj->data_size, GC_TEST | GC_DATA);
	obj->arr_size = length;
	
	memset(obj->data, 0, obj->data_size);
	
	return obj;
}

void utf8_convert(char *utf8, unsigned short *buf) {
	while (*utf8) {
	    int x = *utf8++;          
    	if (x == 0) {
    		break;
    	}
    	                         
    	if(x & 0x80) {                                    
	        int y = *utf8++;
	                                       
        	if(x & 0x20) {                                
            	int z = *utf8++;                           
            	*buf++ = ((x&0xf)<<12)+((y&0x3f)<<6)+(z&0x3f); 
        	} else                                        
            	*buf++ = ((x&0x1f)<<6)+(y&0x3f);               
    	} else *buf++ = x;
	}
}

Object_t * object_new_string(char *str) {
	Exec_t *exec = exec_get_current();
	Object_t *array = object_new_array(T_CHAR, strlen(str));
	
	utf8_convert(str, (unsigned short *)array->data);
	
	Class_t *class = class_resolve("java/lang/String");
	Object_t *string = object_new(class);
	
	*(exec->sp++) = (int)string;
	*(exec->sp++) = (int)array;
	
	MethodInfo_t *constructor = method_resolve(class, utf8_hash("<init>") ^ utf8_hash("([C)V"));
	method_invoke(class, string, constructor);
	
	return string;
}

Object_t * object_new_exception(Class_t *class, char *str) {
	Exec_t *exec = exec_get_current();
	Object_t *message = object_new_string(str);
	Object_t *exception = object_new(class);
	
	PUSH_OBJECT(exception);
	PUSH_OBJECT(message);
	
	MethodInfo_t *constructor = method_resolve(exception->class, utf8_hash("<init>") ^ utf8_hash("(Ljava/lang/String;)V"));
	method_invoke(class, exception, constructor);
	
	return exception;
}

