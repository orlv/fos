#include "alloc.h"
#include "native.h"
#include "heap.h"
#include "metadata/class.h"
#include "exec.h"
#include "synch.h"
#include "thread.h"
#include "gc.h"

#include <stdio.h>
#include <string.h>

#include <sys/io.h>

void java_lang_Object_wait_IV() {
}

void java_lang_Object_notify_V() {
}

void java_lang_Object_notifyAll_V() {
}

void java_lang_System_arraycopy_V() {
	Exec_t *exec = exec_get_current();
	
	int len = POP_INT();
	int destStart = POP_INT();
	Object_t *dst = POP_OBJECT();
	int srcStart = POP_INT();
	Object_t *src = POP_OBJECT();
	
	if (!len) return;
	
	if (src->arr_type && dst->arr_type) {
		int src_size = src->arr_size ? src->data_size / src->arr_size : 0;
		int dst_size = dst->arr_size ? dst->data_size / dst->arr_size : 0;
		
		if (src_size == dst_size) {
			if ((srcStart + len <= src->arr_size) &&
				(destStart + len <= dst->arr_size)) {
				memcpy(dst->data + destStart * src_size, src->data + srcStart * src_size, len * src_size);		
			}
		} else {
			printf("Array length fail.\n");
		}
	} else {
		printf("Not arrays.\n");
	}
}

void java_lang_System_identifyHashCode_I() {
	// NOTHIN TO DO
}

void java_lang_System_gc_V() {
	Exec_t *exec = exec_get_current();
	
	synch_lock(&(exec->heap->mutex));
	
	gc_pass();
	gc_sweep(exec->heap, GC_WHITE);
	
	synch_unlock(&(exec->heap->mutex));
}

void sys_console_newline() {
	printf("\n");
}

void sys_allocated() {
	Exec_t *exec = exec_get_current();
	
	PUSH_INT(1);
}

void sys_console_print() {
	Exec_t *exec = exec_get_current();
	Object_t *str = POP_OBJECT();
	
	if (!str) {
		synch_lock(&(exec->heap->mutex));
		exec_rise_exception(object_new_exception(class_resolve("java/lang/NullPointerException"), "null"));
		synch_unlock(&(exec->heap->mutex));
		return;
	}
	
	FieldInfo_t *field = field_resolve(str->class, "value");
	
#if VERBOSE
	printf("field_offset=[%i], field_desc=[%s]\n", field->offset, field->signature);
#endif
	
	Object_t *str_array = (Object_t*)*((int*)((int)(str->data) + (int)(field->offset)));
	
	unsigned short int *unistr = (unsigned short int *)(str_array->data);
	
	int i = 0;
	
	while (i < str_array->arr_size) {
		printf("%c", (int)*unistr);
		unistr++;
		i++;
	}
}

void sys_port_inb() {
	Exec_t *exec = exec_get_current();
	unsigned short port = (unsigned short)POP_INT();
	unsigned char result;
	result = inb(port);
	PUSH_INT(result);
}

void sys_port_inw() {
	Exec_t *exec = exec_get_current();
	unsigned short port = (unsigned short)POP_INT();
	unsigned short result;
	result = inw(port);
	PUSH_INT(result);
}

void sys_port_inl() {
	Exec_t *exec = exec_get_current();
	unsigned short port = (unsigned short)POP_INT();
	unsigned int result;
	result = inl(port);
	PUSH_INT(result);
}

void sys_port_outb() {
	Exec_t *exec = exec_get_current();
	unsigned short port = (unsigned short)POP_INT();
	unsigned char val = (unsigned char)POP_INT();
	
	outb(val, port);
}

void sys_port_outw() {
	Exec_t *exec = exec_get_current();
	unsigned short port = (unsigned short)POP_INT();
	unsigned short val = (unsigned short)POP_INT();
	
	outw(val, port);
}

void sys_port_outl() {
	Exec_t *exec = exec_get_current();
	unsigned short port = (unsigned short)POP_INT();
	unsigned int val = (unsigned int)POP_INT();
	
	outl(val, port);
}

int kbd_fd = -1;

void sys_console_readchar() {
	Exec_t *exec = exec_get_current();
	
	#if 1
	
	if (kbd_fd == -1) {
		while ((kbd_fd = open("/dev/keyboard", 0)) == -1);
			//sched_yield();
	}
	
	char ch;
	
	read(kbd_fd, &ch, 1);
	
	PUSH_INT((int)ch);
	
	#endif
}

const static Native_t natives[NUM_NATIVES] = {
	// java.lang.Object
	{ 0x4091ea9d, 0x7e6739, java_lang_Object_wait_IV },
	{ 0x4091ea9d, 0xd3691cca, java_lang_Object_notify_V },
	{ 0x4091ea9d, 0xe3bacef5, java_lang_Object_notifyAll_V },
	
	// java.lang.System
	{ 0x53b358d7, 0xe92f9750, java_lang_System_arraycopy_V },
	{ 0x53b358d7, 0xd36d, java_lang_System_gc_V },
	{ 0x53b358d7, 0xf5cd2cfd, java_lang_System_identifyHashCode_I },
	
	// sys.Console
	{ 0xb1603451, 0x193e30a, sys_console_print },
	{ 0xb1603451, 0x654b235d, sys_console_newline },
	{ 0xb1603451, 0xb191977e, sys_console_readchar },
	
	// sys.Memory
	{ 0x41c3329f, 0x545f253b, sys_allocated },
	
	// sys.Port
	{ 0x30cc55b, 0x22ed5b, sys_port_inb }
};

native_func_t native_resolve(hash_t class, hash_t method) {
	int i;
	
	for (i = 0; i < NUM_NATIVES; i++) {
		if (&natives[i] == NULL) break;
		
		if ((natives[i].hmethod == method) && (natives[i].hclass == class)) {
			return natives[i].native;
		}
	}
	
	return NULL;
}
