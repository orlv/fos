#include <sys/io.h>
#include <types.h>
#include <sched.h>
#include "tty.h"
#include "com.h"

void SetBaud(int baud)
{
  int bgc = (1843200 + 8 * baud - 1) / (16 * baud);

  outb(DLAB, BASE + LCR);
  outb(bgc >> 8, BASE + DLM);
  outb(bgc, BASE + DLL);
  outb(0, BASE + LCR);
}

void out_ch(char ch)
{
  while (!(inb(BASE + LSR) & THRE))
    sched_yield();
  outb(ch, BASE + THR);
}

void tty_init()
{
  outb(0x84, BASE + FCR);
  SetBaud(9600);
  outb(Wls8, BASE + LCR);
//FIXME : бывают конфликты с ядром
}

size_t tty_write(off_t offset, const void *buf, size_t count)
{
  for (size_t i = 0; i < count; i++)
    out_ch(((const char *)buf)[i]);
  return count;
}
