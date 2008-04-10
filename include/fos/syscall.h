/*
  include/fos/syscall.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_SYSCALL_H
#define _FOS_SYSCALL_H

#include <types.h>

/* основные системные вызовы -- обмен сообщениями */
#define _FOS_SEND                  1
#define _FOS_RECEIVE               2
#define _FOS_REPLY                 3
#define _FOS_FORWARD               4

/*
  следующие функции целесообразнее разместить
  в системных вызовах -- очень существенно сказавается
  на производительности
*/
#define _FOS_MASK_INTERRUPT        5
#define _FOS_UNMASK_INTERRUPT      6
#define _FOS_SCHED_YIELD           7
#define _FOS_UPTIME                8
#define _FOS_ALARM                 9
#define _FOS_MYTID                 10  /* позволяет потоку узнать свой Thread ID */
#define _FOS_GET_PAGE_PHYS_ADDR    11  /* возвращает физический адрес страницы   */

static inline u32_t sys_call(volatile u32_t cmd, volatile u32_t arg)
{
  u32_t result;
  __asm__ __volatile__ ("int $0xFD":"=a"(result):"b"(cmd), "c"(arg));
  return result;
}

static inline u32_t sys_call2(volatile u32_t cmd, volatile u32_t arg1, volatile u32_t arg2)
{
  u32_t result;
  __asm__ __volatile__ ("int $0xFD":"=a"(result):"b"(cmd), "c"(arg1), "d"(arg2));
  return result;
}
#endif
