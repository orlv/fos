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
static inline int isalpha(int ch) {
  return (unsigned int)((ch | 0x20) - 'a') < 26u;
}

static inline int isdigit ( int ch ) {
    return (unsigned int)(ch - '0') < 10u;
}


static inline int isalnum(int ch) {
  return (unsigned int)((ch | 0x20) - 'a') < 26u  ||
	 (unsigned int)( ch         - '0') < 10u;
}

#endif
 
