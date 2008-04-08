/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef IPC_H
#define IPC_H

/* IPC
  WIN_CMD_WAIT_EVENT -- ожидание события, локального или разрешенного глобального
    arg[..] не изменяются, возврат -- буфер event_q_t
  WIN_CMD_SET_EVMASK -- установка маски глобальных событий
    на входе arg[1] -- новая маска
  WIN_CMD_GET_EVMASK -- получение маски глобальных событий
    на выходе arg[0] -- маска
 */

/* События
  Для обычных событий 
   ev_class -- тип события
   global   -- 0
   handle   -- хендл окна
   a[0:4]   -- аргументы
  Для глобальных
   ev_class -- EV_GLOBAL
   global   -- тип события
   handle   -- 0
   a[0;4]   -- аргументы
 */

/* Типы глобальных событий
  EVG_MMOVE -- движение мыши
   a0       -- x
   a1       -- y
   a2       -- delta x	(нормализован)
   a3       -- delta y	(нормализован)
 */


#define WIN_CMD_WAIT_EVENT (BASE_METHOD_N + 0)
#define WIN_CMD_SET_EVMASK (BASE_METHOD_N + 1)
#define WIN_CMD_GET_EVMASK (BASE_METHOD_N + 2)
#define WIN_CMD_INTERNAL_NOTIFY (BASE_METHOD_N + 3)
#define EV_GLOBAL	1

#define EVG_MMOVE	(1 << 0)


void PostEvent(	tid_t tid, unsigned int handle,	unsigned int ev_class,	unsigned int global,
		unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3);
void ipc_init();


#endif
