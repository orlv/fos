#ifndef TTY_H
#define TTY_H

userlinkage void tty_init();
userlinkage size_t tty_write(off_t offset, const void *buf, size_t count);

#endif
