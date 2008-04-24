/*
 * include/assert.h
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef ASSERT_H
#define ASSERT_H

#ifdef NDEBUG
	#define assert(expr) ((void) 0)
#else
	#ifdef __PRETTY_FUNCTION__
		#define __ASSERT_FUNCTION __PRETTY_FUNCTION__
	#else
		#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
			#define __ASSERT_FUNCTION	__func__
		#else
			#define __ASSERT_FUNCTION	((const char *) 0)
		#endif
	#endif

	userlinkage void __assert_fail (const char *expr, const char *file, unsigned int line, const char *function)
		__attribute__((__noreturn__));

	#define assert(expr) ((void) ((expr) ? 0 : (__assert_fail(#expr, __FILE__, __LINE__, __ASSERT_FUNCTION), 0)))
#endif

#endif

