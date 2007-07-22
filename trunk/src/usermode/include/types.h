/*
    include/types.h
    Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _TYPES_H
#define _TYPES_H

#include <stddef.h>

typedef unsigned int size_t;
typedef signed int ssize_t;
typedef unsigned int num_t;
typedef unsigned int off_t;
typedef unsigned int offs_t;

typedef unsigned char *p_u8_t;
typedef unsigned short *p_u16_t;
typedef unsigned long *p_u32_t;

typedef void *ptr_t;

typedef char *p_char_t;

typedef char *string;

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;

typedef signed char s8_t;
typedef signed short s16_t;
typedef signed long s32_t;

#ifdef __cplusplus
#define asmlinkage extern "C" __attribute__((regparm(0)))
#else
#define asmlinkage
#endif

typedef u32_t mode_t;
typedef u32_t uid_t;
typedef u32_t gid_t;

typedef signed long res_t;

#define RES_SUCCESS   1
#define RES_FAULT    -1
#define RES_FAULT2   -2
#define RES_FAULT3   -3

typedef u32_t pid_t;
typedef u32_t tid_t; /* Thread ID */
typedef tid_t sid_t; /* Server ID (пока что равняется Thread ID) */

#endif
