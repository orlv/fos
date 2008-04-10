/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef MSR_H
#define MSR_H

#include <types.h>


static inline void WriteMSR(u32_t reg, u32_t low, u32_t high) {
	__asm__ __volatile__("wrmsr"::"a" (low), "c" (reg), "d" (high));
}

static inline void ReadMSR(u32_t reg, u32_t *low, u32_t *high) {
	__asm__ __volatile__("rdmsr":"=a"(*low), "=d"(*high):"c"(reg));
}

#endif
