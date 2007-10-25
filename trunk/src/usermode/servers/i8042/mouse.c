#include <fos/fos.h>
#include <sys/io.h>
#include <stdio.h>
#include "mouse.h"
#include "i8042.h"

#define debug_printf printf

int wheel = 0;
int count = 0;
int x = 0;
int y = 0;
int b = 0;
int dx = 0, dy = 0, dz = 0;
u8_t buf[3];

static int set_sample_rate (int rate)
{
  unsigned char ack;
  
  /* Send a reset and wait for an ack response. */
  i8042_aux_write (KBDK_TYPEMATIC);
  if (! i8042_read (&ack) || ack != KBDR_ACK) 
    return 0;

  i8042_aux_write (rate);
  if (! i8042_read (&ack) || ack != KBDR_ACK) 
    return 0;

  return 1;
}

/*
 * Detect Intellimouse device.
 */
static int detect_wheel ()
{
  int count;
  unsigned char ack;
  
  set_sample_rate (200);
  set_sample_rate (100);
  set_sample_rate (80);
  
  /* Send a reset and wait for an ack response. */
  i8042_aux_write (KBDK_READ_ID);
  if (! i8042_read (&ack) || ack != KBDR_ACK) 
    return 0;

  
  /* Get device id. */
  for (count=0; count<1000; ++count) {
    if (i8042_read (&ack)) 
      if (ack == 3)
	return 1;
      return 0;
  }
  return 0;
}


static void
make_move ()
{
  b = 0;
  if (buf[0] & 1)
    b |= MOUSE_BTN_LEFT;
  if (buf[0] & 2)
    b |= MOUSE_BTN_RIGHT;
  if (buf[0] & 4)
    b |= MOUSE_BTN_MIDDLE;
  
  if (!(buf[0] & 0x40)) {
    dx = buf[1];
    if (buf[0] & 0x10)
      dx -= 256;
  } else {
    dx = (buf[0] & 0x10) ? -256 : 255;
  }
  if (! (buf[0] & 0x80)) {
    dy = buf[2];
    if (buf[0] & 0x20)
      dy -= 256;
  } else {
    /* Y oferflow */
    dy = (buf[0] & 0x20) ? -256 : 255;
  }
  if (wheel)
    dz = (signed char) buf[3];
  else
    dz = 0;
  
  /* Invert vertical axis. */
  dy = dy;
  debug_printf ("mouse: (%d, %d, %d) %d\n", dx, dy, dz, b);
}

/*
 * Process a byte, received from mouse.
 * Return 1 when new move record is generated.
 */
void receive_byte (unsigned char byte)
{
  if (count == 0 && ! (byte & 8))
    return;

  buf [count++] = byte;
  if (count == 1 && buf[0] == KBDR_ACK) {
    /* Ignore excess ACK. */
    count = 0;
    return;
  }
  if (count == 2 && buf[0] == KBDR_TEST_OK && buf[1] == 0) {
    /* New device connected. */
    i8042_aux_enable ();
    count = 0;
    wheel = 0;
    if (detect_wheel ())
      wheel = 1;
    return;
  }
  if (count < 3)
    return;
  if (wheel && count < 4)
    return;
  count = 0;
  
  make_move ();
}

/*
 * Process mouse interrupt.
 * Get raw data and put to buffer. Return 1 when not enough data
 * for a move record.
 * Return 0 when a new move record is generated and
 * a signal to mouse task is needed.
 */
void mouse_ps2_interrupt ()
{
  unsigned char c, sts, strobe;
  sts = inb (KBDC_AT_CTL);
  
  c = inb (KBD_DATA);
  if (! (sts & KBSTS_AUX_DATAVL)){
    unmask_interrupt(12);
    return;
  }
  
  strobe = inb (KBDC_XT_CTL);
  outb (strobe | KBDC_XT_CLEAR, KBDC_XT_CTL);
  outb (strobe, KBDC_XT_CTL);
  
  receive_byte (c);
  unmask_interrupt(12);
}

void mouse_ps2_init ()
{
  if (i8042_aux_probe ()) {
    i8042_aux_enable ();
    if (detect_wheel ()) {
      printf("Wheel mouse.\n");
      wheel = 1;
    }
  }
}
