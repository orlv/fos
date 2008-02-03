/*
 * Copyright (c) 2008 Sergey Gridassov
 */

#ifndef READLINE_H
#define READLINE_H
typedef struct {
	int historylen;
	int historyptr;
	char *history[];
} readline_context;

readline_context * readline_init(int historylen);

int readline(char *buf, int size, readline_context *c);

int readline_free(readline_context *c);
#endif
