/*
  drivers/char/usertty/usertty.h
  Copyright (C) 2007 Oleg Fedorov
*/

#ifndef _USERTTY_H
#define _USERTTY_H

#include <tinterface.h>

//#include <drivers/char/tty/tty.h>

class USERTTY: public Tinterface {
private:
  int tty;

public:
  USERTTY();

  size_t write(off_t offset, const void *buf, size_t count);

  void reset();
};

#endif
