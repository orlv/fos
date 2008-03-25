#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_INBUFSZ	20
#define MOUSE_BTN_LEFT		01
#define MOUSE_BTN_RIGHT		02
#define MOUSE_BTN_MIDDLE	04

userlinkage void mouse_ps2_init();
userlinkage void mouse_ps2_interrupt();
userlinkage void MouseHandlerThread();
userlinkage void mouse_receive_byte(unsigned char byte);

#endif
