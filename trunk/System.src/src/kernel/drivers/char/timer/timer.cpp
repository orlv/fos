/*
  drivers/char/timer/timer.cpp
  Copyright (C) 2006 Oleg Fedorov
*/

#include <io.h>
#include <system.h>
#include <stdio.h>
#include "timer.h"

#define I8253_CNTR0	0x040	/* timer 0 counter port */
#define I8253_CNTR1	0x041	/* timer 1 counter port */
#define I8253_CNTR2	0x042	/* timer 2 counter port */
#define I8253_MODE	0x043	/* timer mode port */

#define I8253_MODE_SEL0		0x00	/* select counter 0 */
#define I8253_MODE_SEL1		0x40	/* select counter 1 */
#define I8253_MODE_SEL2		0x80	/* select counter 2 */
#define I8253_MODE_INTTC	0x00	/* mode 0, intr on terminal cnt */
#define I8253_MODE_ONESHOT	0x02	/* mode 1, one shot */
#define I8253_MODE_RATEGEN	0x04	/* mode 2, rate generator */
#define I8253_MODE_SQWAVE	0x06	/* mode 3, square wave */
#define I8253_MODE_SWSTROBE	0x08	/* mode 4, s/w triggered strobe */
#define I8253_MODE_HWSTROBE	0x0a	/* mode 5, h/w triggered strobe */
#define I8253_MODE_LATCH	0x00	/* latch counter for reading */
#define I8253_MODE_LSB		0x10	/* r/w counter LSB */
#define I8253_MODE_MSB		0x20	/* r/w counter MSB */
#define I8253_MODE_16BIT	0x30	/* r/w counter 16 bits, LSB first */
#define I8253_MODE_BCD		0x01	/* count in BCD */

static inline void EnableTimer()
{
  outportb(0x21, inportb(0x21) & 0xfe);	/* Enable timer */
}

TTime::TTime()
{
  u16_t msec_per_tick = 50;
  u16_t count = (1193182 * msec_per_tick + 500) / 1000;
  outportb(I8253_MODE, I8253_MODE_SEL0 | I8253_MODE_RATEGEN | I8253_MODE_16BIT);
  //outportb(I8253_MODE, I8253_MODE_SEL0 | I8253_MODE_SQWAVE | I8253_MODE_16BIT);

  outportb(I8253_CNTR0, count & 0xff);
  outportb(I8253_CNTR0, count >> 8);

  //  EnableTimer();
}

u32_t uptime()
{
  extern TTime *SysTimer;
  return SysTimer->uptime();
}

u32_t TTime::uptime()
{
  return _uptime;
}

void TTime::tick()
{
  _uptime++;
}

#warning TODO: timer_handler(): Изменить. Такой вариант не походит.
#warning TODO: Попробовать с сообщениями.

asmlinkage void timer_handler(u16_t cs)
{
  extern TTime *SysTimer;
  SysTimer->tick();		/* Считаем время */

  u16_t pid = curPID();
  if (pid == 1) {		/* Если мы в scheduler() */
    asm("incb 0xb8000+156\n" "movb $0x5e,0xb8000+157 ");

    outportb(0x20, 0x20);
    return;
  }
  outportb(0x20, 0x20);
  pause();			/* Передадим управление scheduler() */
}
