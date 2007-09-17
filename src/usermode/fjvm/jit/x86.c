#include "x86.h"
#include "objbuf.h"
#include "../exec.h"
#include "../opcodes.h"
#include "../thread.h"
#include "../method.h"
#include "../gc.h"

static unsigned char encode_reg(X86Reg_t reg) {
	unsigned char ret = 0;
	
	switch (reg) {
		case REG_EAX:
			ret = 0;
			break;
			
		case REG_ECX:
			ret = 1;
			break;
			
		case REG_EDX:
			ret = 2;
			break;
			
		case REG_EBX:
			ret = 3;
			break;
			
		case REG_ESP:
			ret = 4;
			break;
			
		case REG_EBP:
			ret = 5;
			break;
	}
	
	return ret;
}

void x86_emit(ObjBuffer_t *buf, unsigned char data) {
	if (buf->offset < buf->size) {
		buf->buffer[buf->offset++] = data;
	} else {
		printf("JIT code buffer overflow.");
	}
}

void x86_push_reg(ObjBuffer_t *buf, X86Reg_t reg) {
	x86_push_reg(buf, 0x50 + encode_reg(reg));
}

unsigned char x86_mod_rm(unsigned char mod, unsigned char reg, unsigned char rm) {
	return ((mod & 0x3) << 6) | ((reg & 0x7) << 3) | (rm & 0x7); 
}

void x86_move_reg(ObjBuffer_t *buf, X86Reg_t dst, X86Reg_t src) {
	unsigned char mod_rm = x86_mod_rm(0x03, encode_reg(src), encode_reg(dst));
	
	x86_emit(buf, 0x89);
	x86_emit(buf, mod_rm);
}

void x86_ret(ObjBuffer_t *buf) {
	x86_emit(buf, 0xc3);
}

void x86_call(ObjBuffer_t *buf, void *func) {
	x86_emit(buf, 0x9a);
	x86_emit(buf, func);
	x86_emit(buf, 0);
	x86_emit(buf, 0);
	x86_emit(buf, 0);
}

ObjBuffer_t * translate(MethodInfo_t *method, Class_t *class) { 
	ObjBuffer_t *buf = objbuf_create(1024);
	//void (*fn)(void);
	Frame_t *frame = frame_create(class, method);
	
	bool_t comp = TRUE;
	while (comp) {
		u1 opcode;
		READ_U1(opcode, frame->cp);
		
		printf("jit: opcode: %x(%i)\n", opcode, opcode);
		
		switch (opcode) {
			case OPC_NOP: continue;
			
			case OPC_RETURN:
				x86_ret(buf);
				comp = FALSE;
				break;
			
			default:
				printf("Invalid opcode. Aborting of jit.\n");
				return NULL;
		}
	}
	
	x86_ret(buf);
	
	//fn = (void(*)(void))buf->buffer;
	//(*fn)();
	
	printf("%s.%s!%s jitted\n", class->name, method->name, method->signature);
	return buf;
}