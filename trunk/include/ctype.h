/*
 * include/ctype.h
 */

#ifndef _CTYPE_H_
#define _CTYPE_H_
static inline int isspace(int ch) {
	return (unsigned int)(ch - 9) < 5u || ch == ' ';
}

static inline int isxdigit(int ch) {
	return (unsigned int)( ch         - '0') < 10u  || 
		(unsigned int)((ch | 0x20) - 'a') <  6u;
}
#endif
 
