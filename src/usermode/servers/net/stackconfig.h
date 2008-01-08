#ifndef STACKCONFIG_H
#define STACKCONFIG_H
/* 
 * Настройки стека.
 * Здесь - дефолтные значения,
 * трогать здесь что либо нежелательно.
 */

/*
 * Количество записей в кеше ARP
 * По умолчанию: 64
 */

#define ARP_CACHE_SIZE	64

/*
 * Таймаут записи в кеше ARP (в миллисекундах)
 * По умолчанию - 2 * 60 * 1000 (2 минуты)
 */

#define ARP_TIMEOUT	(2 * 60 * 1000)

/*
 * Отладка сетевой карты
 * Поведение зависит от драйвера.
 *  Realtek 8139 выводит информацию о пакетах.
 * По умолчанию - отключено (#undef CARD_DEBUG)
 */
#undef CARD_DEBUG
#endif
