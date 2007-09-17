#include "metadata/class.h"
#include "exec.h"
#include "opcodes.h"
#include "thread.h"
#include "method.h"
#include "gc.h"
#include "jit/objbuf.h"
#include "jit/x86.h"

#include <stdio.h>
#include <string.h>

void method_invoke(Class_t *class, Object_t *object, MethodInfo_t *method) {
	#if VERBOSE
	printf("Invoke: %s\n", method->name);
	#endif
	
	void (*fn)(void);
	Class_t *cl = class;
	
	//JavaThread_t *jthread = java_thread_current();
	Exec_t *exec = exec_get_current();
	Frame_t *frame;
	
	/*method->jited = translate(method, cl);
	if (method->jited) {
		fn = (void(*)(void))method->jited->buffer;
		(*fn)();
	}
	*/
	
	if (!(method->access_flags & ACC_NATIVE))  {
		frame = frame_create(cl, method);
	}
	
	if (method->access_flags & ACC_SYNCHRONIZED) {
		synch_lock(&(method->mutex));
		if (object) {
			synch_lock(&(object->mutex));
		}
	}
	
	if (method->access_flags & ACC_NATIVE) {
		#if VERBOSE
		printf("Native method call: %x\n", method->native);
		#endif
		
		method->native();
		
		if (method->access_flags & ACC_SYNCHRONIZED) {
			synch_unlock(&(method->mutex));
			if (object) {
				synch_unlock(&(object->mutex));
			}
		}
		
		return;
	}
	
	ConstPool_t *cp = cl->cp;
	//MethodInfo_t *methods = cl->methods;
	
	int sp_limit = (int)exec->sp;
	
	bool_t running = TRUE;
	
	char index1;
	short int index2;
	//int index4;
	
	Object_t *obj;
	while (running) {
		if (exec->exception) {
			int ex;
			bool_t is_handler = FALSE;
			
			int _cp = frame->cp - method->code;
			
			for (ex = 0; ex < method->exception_table_length; ex++) {
				ExceptionInfo_t *exception_info = &(method->exceptions[ex]);
				
				if ( ((int)(exception_info->start_pc) <= _cp) &&
					 ((int)(exception_info->end_pc) >= _cp)) {
					 	
					 int class_index = exception_info->catch_type;
					 if (class_index == 0) {
					 	is_handler = TRUE;
					 	PUSH_OBJECT(exec->exception);
					 	
					 	frame->cp = (u1*)((int)(method->code + exception_info->handler_pc));
					 	exec->exception = NULL;
					 	break;
					 }
					 
					 int name_index = ((ConstClass_t*)cp->pool[exception_info->catch_type]->data)->nameID;
					 char *class_name = constpool_get_string(cp, name_index);
					 
					 Class_t *handler_class = class_resolve(class_name);
					 
					 if (class_is_super(exec->exception->class, handler_class)) {
					 	is_handler = TRUE;
					 	
					 	PUSH_OBJECT(exec->exception);
					 	frame->cp = (u1*)((int)(method->code + exception_info->handler_pc));
					 	exec->exception = NULL;
					 	break;
					 }
				}
			}
			
			if (!is_handler) {
				printf("Unhandled exception: %s at %s#%s!%s:%i\n", 
					exec->exception->class->name,
					cl->name,
					method->name, 
					method->signature,
					_cp
				);
				running = FALSE;
				break;
			}
		}
		
		if ((int)exec->sp < sp_limit) {
			printf("Stack failure\n");
			
			//running = FALSE;
			*exec->sp = 0;
			exec_rise_exception(object_new_exception(class_resolve("java/lang/RuntimeException"), "Stack overflow."));
			
			break;
		}
		
		u1 opcode;
		READ_U1(opcode, frame->cp);
		
		#if VERBOSE
		printf("%s#%s!%s, opcode: %x(%i)\n", cl->name, method->name, method->signature, opcode, opcode);
		printf("ip->%i\tsp->%i\n", (frame->cp - method->code), exec->sp);
		#endif
		
		switch (opcode) {
			case OPC_NOP:
				break;
				
			case OPC_ACONST_NULL:
				*(exec->sp++) = (int)NULL;
				break;
				
			case OPC_LDC:
				READ_U1(index1, frame->cp);
				
			case OPC_LDC_W:
				if (opcode == OPC_LDC) {
					index2 = index1;	
				} else {
					READ_U2(index2, frame->cp);
				}
				
				switch (cp->pool[index2]->tag) {
					case CONST_INTEGER:
					case CONST_FLOAT:
						PUSH_INT(((ConstInteger_t*)cp->pool[index2]->data)->value);
						break;
						
					case CONST_LONG:
					case CONST_DOUBLE:
						PUSH_LONG(((ConstInteger_t*)cp->pool[index2]->data)->value);
						break;
						
					case CONST_STRING:
						index2 = ((ConstString_t*)cp->pool[index2]->data)->stringID;
						synch_lock(&(exec->heap->mutex));
						
						obj = object_new_string(constpool_get_string(cp, index2));
						*(exec->sp++) = (int)obj;
						gc_mark(exec->heap, GC_TEST, 0, GC_GREY);
						
						synch_unlock(&(exec->heap->mutex));
						break;
						
					default:
						synch_lock(&(exec->heap->mutex));
						exec_rise_exception(object_new_exception(class_resolve("java/lang/RuntimeException"), "Unknow const type"));
						synch_unlock(&(exec->heap->mutex));
						break;
				}
				break;
				
			case OPC_LDC2_W: {
				READ_U2(index2, frame->cp);
				PUSH_LONG(((ConstLong_t*)cp->pool[index2]->data)->value);
				break;
			}
			
			case OPC_ICONST_M1: {
				PUSH_INT(-1);
				break;
			}
			
			case OPC_ICONST_0:
			case OPC_ICONST_1:
			case OPC_ICONST_2:
			case OPC_ICONST_3:
			case OPC_ICONST_4:
			case OPC_ICONST_5:
				PUSH_INT((int)(opcode - OPC_ICONST_0));
				break;
				
			case OPC_LCONST_0:
				PUSH_LONG((long long int)0);
				break;
			
			case OPC_LCONST_1:
				PUSH_LONG((long long int)1);
				break;
				
			case OPC_BIPUSH:
				READ_U1(index1, frame->cp);
				PUSH_INT(index1);
				break;
				
			case OPC_SIPUSH:
				READ_U2(index2, frame->cp);
				PUSH_INT(index2);
				break;
				
			case OPC_ILOAD_0:
			case OPC_ILOAD_1:
			case OPC_ILOAD_2:
			case OPC_ILOAD_3:
				PUSH_INT(frame->locals[opcode - OPC_ILOAD_0]);
				break;
				
			case OPC_ALOAD_0:
			case OPC_ALOAD_1:
			case OPC_ALOAD_2:
			case OPC_ALOAD_3:
				PUSH_OBJECT(frame->locals[opcode - OPC_ALOAD_0]);
				break;
			
			case OPC_IALOAD:
			case OPC_AALOAD: {
				int index = POP_INT();
				obj = POP_OBJECT();
				
				if ( index < 0 || index > obj->arr_size) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/ArrayIndexOutOfBoundsException"), "Index is out of bounds."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					PUSH_INT(((unsigned int*)(obj->data))[index]);
				}
				break;
			}
			
			case OPC_BALOAD: {
				int index = POP_INT();
				obj = POP_OBJECT();
				
				if ( index < 0 || index > obj->arr_size) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/ArrayIndexOutOfBoundsException"), "Index is out of bounds."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					PUSH_INT(((unsigned char*)(obj->data))[index]);
				}
				break;
			}	
			
			case OPC_SALOAD:
			case OPC_CALOAD: {
				int index = POP_INT();
				obj = POP_OBJECT();
				
				if ( index < 0 || index > obj->arr_size) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/ArrayIndexOutOfBoundsException"), "Index is out of bounds."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					PUSH_INT(((unsigned short int*)(obj->data))[index]);
				}
				break;
			}
			
			case OPC_ISTORE:
				READ_U1(index1, frame->cp);
				frame->locals[(int)index1] = POP_INT();
				break;
				
			case OPC_LSTORE:
				READ_U1(index1, frame->cp);
				frame->locals[(int)index1] = POP_INT();
				frame->locals[(int)index1+1] = POP_INT();
				break;
				
			case OPC_ISTORE_0:
			case OPC_ISTORE_1:
			case OPC_ISTORE_2:
			case OPC_ISTORE_3:
				frame->locals[opcode - OPC_ISTORE_0] = POP_INT();
				break;
				
			case OPC_LSTORE_0:
			case OPC_LSTORE_1:
			case OPC_LSTORE_2:
			case OPC_LSTORE_3:
				frame->locals[opcode - OPC_ISTORE_0] = POP_INT();
				frame->locals[opcode - OPC_ISTORE_0 + 1] = POP_INT();
				break;
				
			case OPC_ILOAD:
			case OPC_ALOAD:
			case OPC_FLOAD:
				READ_U1(index1, frame->cp);
				PUSH_INT(frame->locals[(int)index1]);
				break;
				
			case OPC_LLOAD:
				READ_U1(index1, frame->cp);
				PUSH_INT(frame->locals[(int)index1]);
				PUSH_INT(frame->locals[(int)index1 + 1]);
				break;
				
			case OPC_LLOAD_0:
			case OPC_LLOAD_1:
			case OPC_LLOAD_2:
			case OPC_LLOAD_3:
				PUSH_INT(frame->locals[opcode - OPC_LLOAD_0]);
				PUSH_INT(frame->locals[opcode - OPC_LLOAD_0 + 1]);
				break;
				
			case OPC_ASTORE:
				READ_U1(index1, frame->cp);
				frame->locals[(int)index1] = POP_INT();
				break;
				
			case OPC_ASTORE_0:
			case OPC_ASTORE_1:
			case OPC_ASTORE_2:
			case OPC_ASTORE_3:
				frame->locals[opcode - OPC_ASTORE_0] = POP_INT();
				break;
				
			case OPC_IASTORE:
			case OPC_AASTORE: {
				int value = POP_INT();
				int index = POP_INT();
				
				obj = POP_OBJECT();
				
				if ( index < 0 || index > obj->arr_size) {
					frame->cp--;
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/ArrayIndexOutOfBoundsException"), "Index is out of bounds."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					((int*)(obj->data))[index] = value;
				}
				
				break;
			}
			
			case OPC_BASTORE: {
				int value = POP_INT();
				int index = POP_INT();
				
				obj = POP_OBJECT();
				
				if ( index < 0 || index > obj->arr_size) {
					frame->cp--;
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/ArrayIndexOutOfBoundsException"), "Index is out of bounds."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					((char*)(obj->data))[index] = (char)value;
				}
				
				break;
			}
			
			case OPC_CASTORE: {
				int value = POP_INT();
				int index = POP_INT();
				
				obj = POP_OBJECT();
				
				if ( index < 0 || index > obj->arr_size) {
					frame->cp--;
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/ArrayIndexOutOfBoundsException"), "Index is out of bounds."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					((unsigned short int*)(obj->data))[index] = (unsigned short int)value;
				}
				
				break;
			}
			
			case OPC_SWAP: {
				int a = POP_INT();
				int b = POP_INT();
				PUSH_INT(a);
				PUSH_INT(b);
				break;
			}
			
			case OPC_POP2:
				exec->sp--;
			
			case OPC_POP:
				exec->sp--;
				break;
				
			case OPC_DUP:
				*(exec->sp++) = *(exec->sp - 1);
				break;
				
			case OPC_IADD:
				*(exec->sp - 2) += *(exec->sp - 1);
				
				exec->sp--;
				break;
				
			case OPC_ISUB:
				*(exec->sp - 2) -= *(exec->sp - 1);
				exec->sp--;
				break; 
				
				
			case OPC_LSUB: {
				long long int v1 = POP_LONG();
				long long int v2 = POP_LONG();
			
				PUSH_LONG(v2 - v1);
				break;
			}
				
			case OPC_IMUL:
				*(exec->sp - 2) *= *(exec->sp - 1);
				exec->sp--;
				break;  
				
			case OPC_IDIV:
				if (0 == *(exec->sp - 1)) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/Exception"), "Division by zero."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					*(exec->sp - 2) /= *(exec->sp - 1);
				}
				
				exec->sp--;
				break;
				
			case OPC_IREM: {
				int a, b;
				b = POP_INT();
				a = POP_INT();
				
				if (b == 0) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/Exception"), "Division by zero."));
					synch_unlock(&(exec->heap->mutex));
				} else {
					PUSH_INT(a % b);
				}
			}
			
			case OPC_INEG:
				*(exec->sp - 1) = - *(exec->sp - 1);
				break;
				
			case OPC_LNEG: {
				long long int v = POP_LONG();
				PUSH_LONG(-v);
				break;
			}
			
			case OPC_I2B: {
				int v = POP_INT();
				
				if (v < 0) {
					v = -v;
					
					if (v > 128) {
						PUSH_INT(-128);
					} else {
						PUSH_INT(-v);
					}
				} else {
					if (v > 127) {
						PUSH_INT(127);
					} else {
						PUSH_INT(v);
					}
				}
				break;
			}
			
			case OPC_IINC: {
				u1 index;
				char inc;
				READ_U1(index, frame->cp);
				
				inc = *((char*)frame->cp++);
				frame->locals[index] += (int)inc;
			}
			
			case OPC_I2S:
			case OPC_I2C:
				*(exec->sp-1) &= 0xFFFF;
				break;
				
			case OPC_I2L: {
				long long int i = POP_INT();
				PUSH_LONG(i);
				break; 
			}
			
			case OPC_L2I: {
				long long int i = POP_LONG();
				PUSH_INT((int)i);
				break; 
			}
			
			case OPC_LCMP: {
				long long int b = POP_LONG();
				long long int a = POP_LONG();
				
				if (a > b) {
					PUSH_INT(1);
				} else if (a > b) {
					PUSH_INT(-1);
				} else {
					PUSH_INT(0);
				}
				break;
			}
			
			case OPC_IFEQ:
				READ_U2(index2, frame->cp);
				if (POP_INT() == 0) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
				
			case OPC_IFNE:
				READ_U2(index2, frame->cp);
				if (POP_INT() != 0) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
				
			case OPC_IFLT:
				READ_U2(index2, frame->cp);
				if (POP_INT() < 0) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
				
			case OPC_IFGE:
				READ_U2(index2, frame->cp);
				if (POP_INT() >= 0) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
				
			case OPC_IFGT:
				READ_U2(index2, frame->cp);
				if (POP_INT() > 0) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
				
			case OPC_IFLE:
				READ_U2(index2, frame->cp);
				if (POP_INT() <= 0) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
				
			case OPC_IF_ACMPEQ:
			case OPC_IF_ICMPEQ: {
				READ_U2(index2, frame->cp);
				
				int b = POP_INT();
				int a = POP_INT();
				
				if (a == b) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
			}
			
			case OPC_IF_ACMPNE:
			case OPC_IF_ICMPNE: {
				READ_U2(index2, frame->cp);
				
				int b = POP_INT();
				int a = POP_INT();
				
				if (a != b) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
			}
			
			case OPC_IF_ICMPLE: {
				READ_U2(index2, frame->cp);
				
				int b = POP_INT();
				int a = POP_INT();
				
				if (a <= b) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
			}
			
			case OPC_IF_ICMPLT: {
				READ_U2(index2, frame->cp);
				
				int b = POP_INT();
				int a = POP_INT();
				
				if (a < b) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
			}
			
			case OPC_IF_ICMPGT: {
				READ_U2(index2, frame->cp);
				
				int b = POP_INT();
				int a = POP_INT();
				
				if (a > b) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
			}
			
			case OPC_IF_ICMPGE: {
				READ_U2(index2, frame->cp);
				
				int b = POP_INT();
				int a = POP_INT();
				
				if (a >= b) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
			}
			
			case OPC_GOTO: {
				READ_U2(index2, frame->cp);
				frame->cp = frame->cp + index2 - 3;
				break;
			}
			
			case OPC_LOOKUPSWITCH: {
				int old_cp = (int)frame->cp - 1;
				int padding = (frame->cp - frame->method->code + 2) % 4;
				frame->cp = frame->cp + padding;
				
				int def;
				int n_cases;
				
				int key = POP_INT();
				
				READ_U4(def, frame->cp);
				READ_U4(n_cases, frame->cp);
				while (n_cases > 0) {
					int match;
					int offset;
					
					READ_U4(match, frame->cp);
					READ_U4(offset, frame->cp);
					
					if (key == match) {
						frame->cp = (u1*)(old_cp + offset);
						//break;
					}
				}
				
				if (n_cases == 0) {
					frame->cp = (u1*)(old_cp + def);
				}
				break;
			}
			
			case OPC_IRETURN:
				frame->locals[0] = *(exec->sp - 1);
				frame->locals++;
				running = FALSE;
				break;

			case OPC_LRETURN:
				frame->locals[0] = *(exec->sp - 1);
				frame->locals[1] = *(exec->sp - 2);
				frame->locals += 2;
				running = FALSE;
				break;
				
			case OPC_ARETURN:
				obj = POP_OBJECT();
				frame->locals[0] = (int)obj;
				frame->locals++;
				running = FALSE;
				break;
				
			case OPC_RETURN:
				running = FALSE;
				break;
				
			case OPC_GETSTATIC: {
				READ_U2(index2, frame->cp);
				
				ConstFieldref_t *fieldref = (ConstFieldref_t*)(cp->pool[(int)index2]->data);
				int name_index = ((ConstSign_t*)(cp->pool[(int)fieldref->signID]->data))->nameID;
				
				char *field_name = constpool_get_string(cp, name_index);
				char *class_name = constpool_get_string(cp, ((ConstClass_t*)(cp->pool[fieldref->classID]->data))->nameID);
				
				Class_t *dst_class;
				
				if (strcmp(class_name, cl->name) == 0) {
					dst_class = cl;
				} else {
					dst_class = class_resolve(class_name);
				}
				
				FieldInfo_t *res_field = field_resolve(dst_class, field_name);
				
				if (res_field->signature[0] == 'J' || res_field->signature[0] == 'D') {
					PUSH_INT(res_field->offset);
				}
				PUSH_INT(res_field->static_value);
				break;
			}
			
			case OPC_PUTSTATIC: {
				READ_U2(index2, frame->cp);
				
				ConstFieldref_t *fieldref = (ConstFieldref_t*)(cp->pool[(int)index2]->data);
				int name_index = ((ConstSign_t*)(cp->pool[(int)fieldref->signID]->data))->nameID;
				
				char *field_name = constpool_get_string(cp, name_index);
				char *class_name = constpool_get_string(cp, ((ConstClass_t*)(cp->pool[fieldref->classID]->data))->nameID);
				
				Class_t *dst_class;
				
				if (strcmp(class_name, cl->name) == 0) {
					dst_class = cl;
				} else {
					dst_class = class_resolve(class_name);
				}
				
				FieldInfo_t *res_field = field_resolve(dst_class, field_name);
				
				res_field->static_value = (void*)POP_INT();
				if (res_field->signature[0] == 'J' || res_field->signature[0] == 'D') {
					res_field->offset = POP_INT();
				}
				
				break;
			}
			
			case OPC_INVOKEINTERFACE: {
				READ_U2(index2, frame->cp);
				
				u1 nargs;
				u1 reserved;
				
				READ_U1(nargs, frame->cp);
				READ_U1(reserved, frame->cp);
				
				ConstMethodref_t *methodref = constpool_get_method(cp, index2);
				//char *class_name = constpool_get_string(cp, constpool_get_class(cp, methodref->classID)->nameID);
				char *method_name = constpool_get_string(cp, constpool_get_signature(cp, methodref->signID)->nameID);
				char *type_desc = constpool_get_string(cp, constpool_get_signature(cp, methodref->signID)->descID);
				
				obj = (Object_t*)*(exec->sp - nargs);
				
				if (obj == NULL) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
					synch_unlock(&(exec->heap->mutex)); 
				} else {
					Class_t *inv_class = class_method_resolve(obj->class, utf8_hash(method_name) ^ utf8_hash(type_desc));
					MethodInfo_t *inv_method = method_resolve(inv_class, utf8_hash(method_name) ^ utf8_hash(type_desc));
					
					method_invoke(inv_class, obj, inv_method);
				}
				break;
			}
			
			case OPC_INVOKEVIRTUAL: {
				READ_U2(index2, frame->cp);
				ConstMethodref_t *methodref = constpool_get_method(cp, index2);
				
				int idx = methodref->index;
				int nargs = methodref->arg_count;
				
				hash_t hash;
				if (idx != 0) {
					obj = (Object_t*)*(exec->sp - nargs);
					if (obj == NULL) {
						synch_lock(&(exec->heap->mutex));
						exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
						synch_unlock(&(exec->heap->mutex));
					} else {
						method_invoke(obj->class->vmt[idx]->owner, obj, obj->class->vmt[idx]);
					}
				} else {
					char *class_name = constpool_get_string(cp, constpool_get_class(cp, methodref->classID)->nameID);
					char *method_name = constpool_get_string(cp, constpool_get_signature(cp, methodref->signID)->nameID);
					char *type_desc = constpool_get_string(cp, constpool_get_signature(cp, methodref->signID)->descID);
					
					Class_t *inv_class = class_resolve(class_name);
					hash = utf8_hash(method_name) ^ utf8_hash(type_desc);
					
					MethodInfo_t *inv_method = method_resolve(inv_class, hash);
					
					nargs = inv_method->arg_count;
					obj = (Object_t*)*(exec->sp - nargs);
					if (NULL == obj) {
						synch_lock(&(exec->heap->mutex));
						exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
						synch_unlock(&(exec->heap->mutex));
					} else {
						inv_class = class_method_resolve(obj->class, hash);
						MethodInfo_t *inv_method = method_resolve(inv_class, hash);
						
						methodref->index = inv_method->index;
						methodref->arg_count = nargs;
						
						// WARN
						obj->class->vmt[inv_method->index]->owner = inv_class;
						
						method_invoke(inv_class, obj, inv_method);
					}
				}
				break;
			}
			
			case OPC_INVOKESPECIAL:
			case OPC_INVOKESTATIC: {
				READ_U2(index2, frame->cp);
				ConstMethodref_t *methodref = constpool_get_method(cp, index2);
				
				char *class_name = constpool_get_string(cp, constpool_get_class(cp, methodref->classID)->nameID);
				char *method_name = constpool_get_string(cp, constpool_get_signature(cp, methodref->signID)->nameID);
				char *type_desc = constpool_get_string(cp, constpool_get_signature(cp, methodref->signID)->descID);
					
				Class_t *inv_class = class_resolve(class_name);
				MethodInfo_t *inv_method = method_resolve(inv_class, utf8_hash(method_name) ^ utf8_hash(type_desc));
				if (OPC_INVOKESTATIC == opcode) {
					inv_class = class_method_resolve(inv_class, utf8_hash(method_name) ^ utf8_hash(type_desc));
				}
				
				method_invoke(inv_class, NULL, inv_method);
				break;
			}
				
			case OPC_GETFIELD: {
				READ_U2(index2, frame->cp);
				obj = POP_OBJECT();
				
				if (NULL == obj) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
					synch_unlock(&(exec->heap->mutex));
				}
				
				ConstFieldref_t *fieldref = constpool_get_field(cp, index2);
				FieldInfo_t *inv_field = fieldref->field;
				if (!inv_field) {
					char *class_name = constpool_get_string(cp, constpool_get_class(cp, fieldref->classID)->nameID);
					char *field_name = constpool_get_string(cp, constpool_get_signature(cp, fieldref->signID)->nameID);
					
					Class_t *inv_class = class_resolve(class_name);
					inv_field = field_resolve(inv_class, field_name);
					fieldref->field = inv_field;
				}
				
				switch (inv_field->signature[0]) {
					case 'Z':
					case 'B':
						PUSH_INT(*(char*)(obj->data + inv_field->offset));
						break;
						
					case 'C':
					case 'S':
						PUSH_INT(*(short int*)(obj->data + inv_field->offset));
						break;
						
					case 'F':
					case 'I':
					case '[':
					case 'L':
						PUSH_INT(*(int*)(obj->data + inv_field->offset));
						break;
						
					case 'J':
					case 'D':
						PUSH_LONG(*(long long int*)(obj->data + inv_field->offset));
						break;
					default: break;
				}
				break;
			}
			
			case OPC_PUTFIELD: {
				READ_U2(index2, frame->cp);
				
				ConstFieldref_t *fieldref = constpool_get_field(cp, index2);
				FieldInfo_t *inv_field = fieldref->field;
				if (!inv_field) {
					char *class_name = constpool_get_string(cp, constpool_get_class(cp, fieldref->classID)->nameID);
					char *field_name = constpool_get_string(cp, constpool_get_signature(cp, fieldref->signID)->nameID);
					
					Class_t *inv_class = class_resolve(class_name);
					inv_field = field_resolve(inv_class, field_name);
					fieldref->field = inv_field;
				}
				
				switch (inv_field->signature[0]) {
					case 'Z':
					case 'B':						
					case 'C':
					case 'S':						
					case 'F':
					case 'I':
					case '[':
					case 'L': {
						int value = POP_INT();
						obj = POP_OBJECT();
						
						if (obj == NULL) {
							synch_lock(&(exec->heap->mutex));
							exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
							synch_unlock(&(exec->heap->mutex));
						} else {
							*((int*)(obj->data + inv_field->offset)) = value;
						}
						break;
					}
						
					case 'J':
					case 'D': {
						long long int value = POP_LONG();
						obj = POP_OBJECT();
						
						if (obj == NULL) {
							synch_lock(&(exec->heap->mutex));
							exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
							synch_unlock(&(exec->heap->mutex));
						} else {
							*((long long int*)(obj->data + inv_field->offset)) = value;
						} 
						break;
					}
					
					default: break;
				}
				break;
			}
			
			case OPC_NEW: {
				READ_U2(index2, frame->cp);
				
				char *class_name = constpool_get_string(cp, constpool_get_class(cp, index2)->nameID);
				Class_t *inv_class = class_resolve(class_name);
				obj = NULL;
				
				#if VERBOSE
				printf("Creating class: %s\n", class_name);
				#endif
				
				synch_lock(&(exec->heap->mutex));
				if (inv_class != NULL) {
					obj = object_new(inv_class);
					
					#if VERBOSE
					printf("Created object at: %x\n", (int)obj);
					#endif
				}
				
				PUSH_OBJECT(obj);
				
				gc_mark(exec->heap, GC_TEST, 0, GC_GREY);
				synch_unlock(&(exec->heap->mutex));
				break;
			}
			
			case OPC_NEWARRAY: {
				READ_U1(index2, frame->cp);
				int length = POP_INT();
				synch_lock(&(exec->heap->mutex));
				obj = object_new_array(index2, length);
				PUSH_OBJECT(obj);
				gc_mark(exec->heap, GC_TEST, 0, GC_GREY);
				synch_unlock(&(exec->heap->mutex));
				break;
			}
			
			case OPC_ANEWARRAY: {
				READ_U2(index2, frame->cp);
				int length = POP_INT();
				synch_lock(&(exec->heap->mutex));
				obj = object_new_array(13, length);
				PUSH_OBJECT(obj);
				gc_mark(exec->heap, GC_TEST, 0, GC_GREY);
				synch_unlock(&(exec->heap->mutex));
				break;
			}
			
			case OPC_ARRAYLENGTH: {
				obj = POP_OBJECT();
				if (obj == NULL) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
					synch_unlock(&(exec->heap->mutex));
					break;
				}
				
				if (obj->arr_type) {
					PUSH_INT(obj->arr_size);
				} else {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/RuntimeException"), "arraylength to not array."));
					synch_unlock(&(exec->heap->mutex));
				}
				break;
			}
			
			case OPC_IFNULL:
				READ_U2(index2, frame->cp);
				if (POP_OBJECT() == NULL) {
					frame->cp = frame->cp + index2- 3;
				}
				break;
				
			case OPC_IFNONNULL:
				READ_U2(index2, frame->cp);
				if (POP_OBJECT() != NULL) {
					frame->cp = frame->cp + index2 - 3;
				}
				break;
				
			case OPC_MONITORENTER:
				obj = POP_OBJECT();
				if (obj == NULL) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
					synch_unlock(&(exec->heap->mutex));
				} else {
					synch_lock(&(obj->mutex));
				}
				break;
				
			case OPC_MONITOREXIT:
				obj = POP_OBJECT();
				if (obj == NULL) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
					synch_unlock(&(exec->heap->mutex));
				} else {
					synch_unlock(&(obj->mutex));
				}
				break;
			
			case OPC_INSTANCEOF: {
				READ_U2(index2, frame->cp);
				obj = POP_OBJECT();
				
				if (obj == NULL) {
					PUSH_INT(0);
					break;
				} else {
					char *class_name = constpool_get_string(cp, constpool_get_class(cp, index2)->nameID);
					Class_t *inv_class = class_resolve(class_name);
					
					if (class_is_super(obj->class, inv_class)) {
						PUSH_INT(1);
					} else { 
						PUSH_INT(0);
					}
				}
				break;
			}
			
			case OPC_ATHROW: {
				obj = POP_OBJECT();
				
				if (obj == NULL) {
					synch_lock(&(exec->heap->mutex));
					exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
					synch_unlock(&(exec->heap->mutex));
					break;
				}
				exec_rise_exception(obj);
				break;
			}
			
			case OPC_CHECKCAST: {
				READ_U2(index2, frame->cp);
				obj = (Object_t*)(*(exec->sp-1));
				
				if (obj != NULL) {
					char *class_name = constpool_get_string(cp, constpool_get_class(cp, index2)->nameID);
					Class_t *inv_class = class_resolve(class_name);
					
					if (!class_is_super(obj->class, inv_class)) {
						synch_lock(&(exec->heap->mutex));
						exec_rise_exception(object_new_exception(class_resolve("java/lang/ClassCastException"), "Cannot cast object to class specified."));
						synch_unlock(&(exec->heap->mutex));
					}
				}
				break;
			}
			
			default:
				running = FALSE;
				printf("invalid opcode %i at %s#%s!%s:%i\n", opcode, cl->name, method->name, method->signature, (frame->cp - method->code));
				
				synch_lock(&(exec->heap->mutex));
				exec_rise_exception(object_new_exception(class_resolve("java/lang/RuntimeException"), "Invalid opcode."));
				synch_unlock(&(exec->heap->mutex));
				break;
		}
	}
	
	if (method->access_flags & ACC_SYNCHRONIZED) {
		synch_unlock(&(method->mutex));
		if (object) {
			synch_unlock(&(object->mutex));
		}
	}
	
	frame_delete();
	if (exec->frame == NULL) return;
	
	cl = exec->frame->class;
	method = exec->frame->method;
}

