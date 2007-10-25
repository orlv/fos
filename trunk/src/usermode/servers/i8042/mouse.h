#ifndef MOUSE_H
#define MOUSE_INBUFSZ	20
#define MOUSE_BTN_LEFT		01
#define MOUSE_BTN_RIGHT		02
#define MOUSE_BTN_MIDDLE	04

void mouse_ps2_init ();
void mouse_ps2_interrupt ();
void MouseHandlerThread();
#define MOUSE_H
#endif
