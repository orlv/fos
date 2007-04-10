/*
        kernel/include/io.h
        Copyright (C) 2005 Oleg Fedorov
*/

#ifndef __IO_H
#define __IO_H

inline void outportb(unsigned short port, unsigned char value)
{
  asm volatile ("outb %0,%1"::"a" (value), "d"(port));
};

inline unsigned char inportb(unsigned short port)
{
  unsigned char value;
  asm volatile ("inb %1, %0":"=a" (value):"d"(port));
  return value;
};

inline void outportw(unsigned short port, unsigned short value)
{
  asm volatile ("outw %0,%1"::"a" (value), "d"(port));
};

inline unsigned short inportw(unsigned short port)
{
  unsigned short value;
  asm volatile ("inw %1, %0":"=a" (value):"d"(port));
  return value;
};

#endif
