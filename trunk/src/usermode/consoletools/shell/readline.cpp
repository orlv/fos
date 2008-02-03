/*
 * Copyright (c) 2008 Sergey Gridassov
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "readline.h"


static void readline_add_to_history(char *str, readline_context *c) {
	char *newstr = new char[strlen(str) + 1];
	if(!newstr) return;
	strcpy(newstr, str);
	c->historyptr = c->historylen - 1;
	if(c->history[0]) 
		free(c->history[0]);
	
	memmove((char *)c->history, (char *)c->history + sizeof(char *), (c->historylen - 1) * sizeof(char *));
	c->history[c->historyptr] = newstr;
	c->historyptr++;
}

int readline(char *buf, int size, readline_context *c) {
	buf[0] = 0;
	int entered = 0;
	char *ptr = buf;
	while(1) {
		char ch = getchar();
		switch(ch) {
		case 0x01:
			ch = getchar();
			switch(ch) {
			case 0x4B:
				if(ptr > buf) {
					ptr--;
					printf("\033[1D");
				}
				break;
			case 0x77:
				if(*ptr != 0) {
					ptr++;
					printf("\033[1C");
				}
				break;
			case 0x72:
				if(c->historyptr == c->historylen) {
					readline_add_to_history(buf, c);
					c->historyptr--;
				}
				if(c->historyptr > 0 && c->history[c->historyptr - 1]) {
					c->historyptr--;
					ptr = buf;
					int len = strlen(ptr);
					printf("\033[%uD", len);
					for(int i = 0; i < len; i++) printf(" ");
					printf("\033[%uD", len);
					ptr[0] = 0;
					strncpy(ptr, c->history[c->historyptr], size);
					len = strlen( c->history[c->historyptr]);
					printf("%s", ptr);
					ptr+=len;
					entered = len;
				}
				break;
			case 0x80:
				if(c->historyptr < c->historylen - 1 && c->history[c->historyptr + 1]) {
					c->historyptr++;
					ptr = buf;
					int len = strlen(ptr);
					printf("\033[%uD", len);
					for(int i = 0; i < len; i++) printf(" ");
					printf("\033[%uD", len);
					ptr[0] = 0;
					strncpy(ptr, c->history[c->historyptr], size);
					len = strlen( c->history[c->historyptr]);
					printf("%s", ptr);
					ptr+=len;
					entered = len;
				}
				break;
			default:
			//	printf("Unknown esc %x\n", ch);
				break;
			}
			break;
		case '\n':
			buf[entered] = 0;
			printf("\n");
			readline_add_to_history(buf, c);
			return entered;
		case 0x7F:
		case 8:
			if(entered) {
				entered--;
				printf("\x08 \x08");
				if(*ptr == 0) {
					ptr--;
					*ptr = 0;
				} else {
					ptr--;
					memmove(ptr, ptr + 1, strlen(ptr) + 1);
					printf("%s \x08\033[%uD", ptr, strlen(ptr));
				}
			}
			break;
		default:
			if(entered < size) {
				entered ++;
				printf("%c", ch);
				if(*ptr == 0) {
					*ptr = ch;
					ptr++;
					*ptr = 0;
				} else {
					memmove(ptr + 1, ptr, strlen(ptr) + 1);
					*ptr = ch;
					ptr++;
					printf("%s\033[%uD", ptr, strlen(ptr));
				}
			}
		}
		fflush(stdout);
	}
}

readline_context *readline_init(int historylen) {
	readline_context *rc = (readline_context *)malloc(sizeof(readline_context) + historylen * sizeof(char *));
	if(!rc)
		return NULL;
	rc->historylen = historylen;
	rc->historyptr = historylen - 1;
	for(int i = 0; i < historylen; i++)
		rc->history[i] = NULL;
	return rc;
}

int readline_free(readline_context *c) {
	for(int i = 0; i < c->historylen; i++)
		if(c->history[i])
			free(c->history[i]);
	free(c);
	return 0;
}
