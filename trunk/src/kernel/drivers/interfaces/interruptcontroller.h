/*
  drivers/interfaces/interruptcontroller.h
  Copiright (C) 2008 Sergey Gridassov
*/

#ifndef __INTERRUPTCONTROLLER_H
#define __INTERRUPTCONTROLLER_H

class InterruptController {
public:
	virtual void mask(int n) = 0;
	virtual void unmask(int n) = 0;

	virtual void lock() = 0;
	virtual void unlock() = 0;

	virtual void Route(int n) = 0;
	virtual void setHandler(int n, void *handler) = 0;

	virtual void* getHandler(int n) = 0;

	virtual void EOI(int irq) = 0;;
};

#endif
