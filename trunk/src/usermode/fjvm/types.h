#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>

//typedef unsigned int size_t;    /* object size */
typedef unsigned int count_t;   /* objects counter */

#ifndef FALSE
typedef enum { FALSE = 0, TRUE } bool_t;
#else
typedef int bool_t;
#endif /* ndef FALSE */

#ifndef NULL
#   define NULL ((void*)0)
#endif


typedef unsigned long long int  u8;
typedef unsigned long int       u4;
typedef unsigned short int      u2;
typedef unsigned char           u1;

/**
 * Data fetch macros IA-32 architecture
 */
#define READ_U1(v,p)  v = *p++
#define READ_U2(v,p)  v = (p[0]<<8)|p[1]; p+=2
#define READ_U4(v,p)  v = (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3]; p+=4
#define READ_U8(v,p)  v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8
                            
#define READD_U1(p) *p++
#define READD_U2(p)  (p[0]<<8)|p[1]; p+=2
#define READD_U4(p)  (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3]; p+=4
#define READD_U8(p)  ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8

#endif /*TYPES_H_*/
