/*
  drivers/pic/pic.h
  Copiright (C) 2007 Oleg Fedorov
                2008 Sergey Gridassov
*/

#ifndef _PIC_H
#define _PIC_H
#include <fos/drivers/interfaces/interruptcontroller.h>
#include <types.h>

class PIC: public InterruptController {
  volatile u16_t status;
  void remap(u8_t v1, u8_t v2);
  void (*handlers[15])();
 public:
  PIC();

  void mask(int n);
  void unmask(int n);

  void lock();
  void unlock();

  void Route(int n);
  void setHandler(int n, void *handler);
  void *getHandler(int n);

  void EOI(int irq);
};

#endif
