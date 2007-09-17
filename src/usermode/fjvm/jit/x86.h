#ifndef X86_H_
#define X86_H_

#include "objbuf.h"
#include "../metadata/class.h"

typedef
enum X86Reg_e {
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	
	REG_EBP,
	REG_ESP
} X86Reg_t;

void x86_emit(ObjBuffer_t *buf, unsigned char data);
unsigned char x86_mod_rm(unsigned char mod, unsigned char reg, unsigned char rm);

void x86_push_reg(ObjBuffer_t *buf, X86Reg_t reg);
void x86_move_reg(ObjBuffer_t *buf, X86Reg_t dst, X86Reg_t src);

void x86_ret(ObjBuffer_t *buf);
void x86_call(ObjBuffer_t *buf, void *func);

ObjBuffer_t * translate(MethodInfo_t *method, Class_t *class);

#endif /*X86_H_*/
