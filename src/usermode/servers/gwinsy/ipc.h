/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef IPC_H
#define IPC_H

#define WIN_CMD_WAIT_EVENT 	(BASE_METHOD_N + 0)
#define WIN_CMD_SET_EVMASK 	(BASE_METHOD_N + 1)
#define WIN_CMD_GET_EVMASK 	(BASE_METHOD_N + 2)
#define WIN_CMD_INTERNAL_NOTIFY	(BASE_METHOD_N + 3)
#define WIN_CMD_CREATE_WINDOW 	(BASE_METHOD_N + 4)
#define WIN_CMD_MAP_WINDOW 	(BASE_METHOD_N + 0)
#define WIN_CMD_GET_WINDOW_ATTR	(BASE_METHOD_N + 5)
#define WIN_CMD_SET_WINDOW_ATTR	(BASE_METHOD_N + 6)
#define WIN_CMD_RESIZE		(BASE_METHOD_N + 7)

#define EV_GLOBAL	1
#define EV_MMOVE	2

#define EVG_MMOVE	(1 << 0)
#define EVG_ATTR_MODIFY	(1 << 1)

void PostEvent(	tid_t tid, unsigned int handle,	unsigned int ev_class,	unsigned int global,
		unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3);
void ipc_init();

/* IPC
  WIN_CMD_WAIT_EVENT -- ожидание события, локального или разрешенного глобального
    arg[..] не изменяются, возврат -- буфер event_q_t

  WIN_CMD_SET_EVMASK -- установка маски глобальных событий
    на входе arg[1] -- новая маска

  WIN_CMD_GET_EVMASK -- получение маски глобальных событий
    на выходе arg[0] -- маска

  WIN_CMD_CREATE_WINDOW -- создание окна
    на входе буфер create_win_t, на выходе arg[0] хендл или 0 в случае ошибки

  WIN_CMD_MAP_WINDOW -- мапинг окна
    на входе arg[1] -- хендл и буфер

  WIN_CMD_GET_WINDOW_ATTR -- получение атрибутов окна
    на входе arg[1] -- хендл
    на выходе буфер win_attr_t, код успеха в arg[0] (0 или -1)

  WIN_CMD_SET_WINDOW_ATTR -- установка атрибутов окна
    на входе arg[1] -- хендл, буфер win_attr_t
    на выходе arg[0] код успеха
  Создает событие EVG_ATTR_MODIFY

  WIN_CMD_RESIZE -- изменение размера окна
    на входе arg[1] -- хендл, arg[2] -- ширина, arg[3] -- высота
    на выходе arg[0] -- код успеха
  Создает события EVG_ATTR_MODIFY и EV_RESIZE

* События
  Для обычных событий 
   ev_class -- тип события
   global   -- 0
   handle   -- хендл окна
   a[0:3]   -- аргументы
  Для глобальных
   ev_class -- EV_GLOBAL
   global   -- тип события
   handle   -- 0
   a[0:3]   -- аргументы

* Типы глобальных событий
  EVG_MMOVE -- движение мыши
   a0       -- x
   a1       -- y
   a2       -- delta x	(нормализован)
   a3       -- delta y	(нормализован)
  EVG_ATTR_MODIFY -- изменились атрибуты окна
   a0      -- хендл
   a[1:3]  -- не испльзуются

* Типы обычных событий
  EV_RESIZE -- изменены размеры окна
   a0       -- ширина
   a1       -- высота
   a[2:3]   -- не используются
   Рекомендуется использовать для перерисовки содержимого окна

* СТРУКТУРА WIN_ATTR_T
 unsigned int parent -- окно-родитель
  при изменении -- окно перемещается в списках!
 int x, y -- координаты окна
  окно перемещается соответственно
 unsigned int w, h -- размеры окна
  только для чтения. для ресайза использовать WIN_CMD_RESIZE (из-за сложности операции)
 char title[64] -- заголовок окна
  обновляется соответственно
 int visible -- видимость окна
*/

#endif
