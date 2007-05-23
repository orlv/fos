/*
  Copyright (C) 2005 Oleg Fedorov
*/

#include <stdio.h>
#include <fcntl.h>

int main()
{
	int fd = open("font.fnt", O_RDONLY);
	int out = open("res.fnt", O_WRONLY | O_CREAT| O_TRUNC, 0660);
	int i,j;
	unsigned char ch[16];
	for(i=0; i< (0x1000/16); i++)
	{
		read(fd, ch, 16);
		write(out, ch, 16);
		for(j=0; j<16; j++) ch[j] = 0;
		write(out,ch,16);
	}
	return 0;
}
