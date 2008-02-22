/*
 *         FOS Graphics System
 * Copyright (c) 2008 Sergey Gridassov
 */
#ifndef FIPC_H
#define FIPC_H
void PostEvent(int tid, int handle, int class, int a0, int a1, int a2, int a3);
void StartIPC();
#endif
