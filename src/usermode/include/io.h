/*
        kernel/include/io.h
        Copyright (C) 2005 Oleg Fedorov
*/

#ifndef __IO_H
#define __IO_H

inline void outb(unsigned char value, unsigned short port)
{
  asm volatile ("outb %0,%1"::"a" (value), "d"(port));
};

inline unsigned char inb(unsigned short port)
{
  unsigned char value;
  asm volatile ("inb %1, %0":"=a" (value):"d"(port));
  return value;
};

inline void outw(unsigned short value, unsigned short port)
{
  asm volatile ("outw %0,%1"::"a" (value), "d"(port));
};

inline unsigned short inw(unsigned short port)
{
  unsigned short value;
  asm volatile ("inw %1, %0":"=a" (value):"d"(port));
  return value;
};

#endif
