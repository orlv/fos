/*
  drivers/pic/pic.h
  Copiright (C) 2007 Oleg Fedorov
*/

#ifndef _PIC_H
#define _PIC_H

#include <types.h>

class PIC {
  volatile u16_t status;
  
 public:
  PIC();

  void remap(u8_t v1, u8_t v2);
  void mask(u8_t n);
  void unmask(u8_t n);

  void lock();
  void unlock();
};

#endif
