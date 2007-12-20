/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */
#ifndef EVENTS_H
#define EVENTS_H
void StartEventHandling(void);
void PostEvent(int tid, int handle, int class, int a0, int a1, int a2, int a3);
#endif

