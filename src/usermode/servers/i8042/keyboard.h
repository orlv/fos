#define KB_CHARS_BUFF_SIZE 256
void keyboard_ps2_init ();
void keyboard_ps2_interrupt();
res_t kb_put(u8_t ch);

size_t kb_write(off_t offset, const void *buf, size_t count);
size_t kb_read(off_t offset, void *buf, size_t count);