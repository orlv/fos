/*
 * Copyright (c) 2008 Sergey Gridassov
 */


#include <stdio.h>
#include <string.h>


int readline(char *buf, int size) {
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
			default:
	//			printf("Unknown esc %c\n", ch);
				break;
			}
			break;
		case '\n':
			buf[entered] = 0;
			printf("\n");
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
