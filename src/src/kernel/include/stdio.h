/*
        kernel/include/stdio.h
        Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <types.h>

int printk(register const char *fmt, ...);
int printf(const char *fmt, ...);

/* Vga colors */
#define	BLACK			0
#define	BLUE			1
#define	GREEN			2
#define	CYAN			3
#define	RED			4
#define	MAGENTA			5
#define	BROWN			6
#define	LIGHTGRAY		7
#define	DARKGRAY		8
#define	LIGHTBLUE		9
#define	LIGHTGREEN		10
#define	LIGHTCYAN		11
#define	LIGHTRED		12
#define	LIGHTMAGENTA		13
#define	YELLOW			14
#define	WHITE			15

#endif
