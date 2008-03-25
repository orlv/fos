#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <types.h>

#define KB_CHARS_BUFF_SIZE 256

userlinkage void keyboard_ps2_init();
userlinkage void keyboard_ps2_interrupt();
userlinkage void keyboard_receive_byte(unsigned char scancode);
userlinkage res_t kb_put(u8_t ch);

userlinkage size_t kb_write(off_t offset, const void *buf, size_t count);
userlinkage size_t kb_read(off_t offset, void *buf, size_t count);

#endif
