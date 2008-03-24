/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef ASSERT_H
#define ASSERT_H
void assert_failed(const char *expression, const char *file, int line);
#define assert(expr) { if(!(expr)) assert_failed(#expr, __FILE__, __LINE__); }
#endif
