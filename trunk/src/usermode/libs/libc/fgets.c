/*
 * Copyright (c) 2008 Sergey Gridassocv
 */
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
void fgets(char *buf, size_t size, int handle) {
	char *start;
	start = buf;
	char c;

	do {
		read(handle, &c, 1);
		switch(c) {
		case 8:
			if(start < buf) {
				buf--;
				*buf = 0;
				printf("%c", c);
			}
			break;
		default:
			if((buf - start) < size) {
				*buf = c;
				printf("%c", c);
				buf++;
				*buf = 0;
			}
		}
	} while(c != '\n');
}
