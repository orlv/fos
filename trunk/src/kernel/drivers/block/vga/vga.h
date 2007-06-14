/*
        drivers/block/vga/vga.h
        Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _VGA_H
#define _VGA_H

#include <drivers/char/tty/tty.h>

/* VGA data register ports */
#define VGA_CRT_DC      0x3D5	/* CRT Controller Data Register - color emulation */
#define VGA_CRT_DM      0x3B5	/* CRT Controller Data Register - mono emulation */
#define VGA_ATT_R       0x3C1	/* Attribute Controller Data Read Register */
#define VGA_ATT_W       0x3C0	/* Attribute Controller Data Write Register */
#define VGA_GFX_D       0x3CF	/* Graphics Controller Data Register */
#define VGA_SEQ_D       0x3C5	/* Sequencer Data Register */
#define VGA_MIS_R       0x3CC	/* Misc Output Read Register */
#define VGA_MIS_W       0x3C2	/* Misc Output Write Register */
#define VGA_FTC_R       0x3CA	/* Feature Control Read Register */
#define VGA_IS1_RC      0x3DA	/* Input Status Register 1 - color emulation */
#define VGA_IS1_RM      0x3BA	/* Input Status Register 1 - mono emulation */
#define VGA_PEL_D       0x3C9	/* PEL Data Register */
#define VGA_PEL_MSK     0x3C6	/* PEL mask register */

/* EGA-specific registers */
#define EGA_GFX_E0      0x3CC	/* Graphics enable processor 0 */
#define EGA_GFX_E1      0x3CA	/* Graphics enable processor 1 */

/* VGA index register ports */
#define VGA_CRT_IC      0x3D4	/* CRT Controller Index - color emulation */
#define VGA_CRT_IM      0x3B4	/* CRT Controller Index - mono emulation */
#define VGA_ATT_IW      0x3C0	/* Attribute Controller Index & Data Write Register */
#define VGA_GFX_I       0x3CE	/* Graphics Controller Index */
#define VGA_SEQ_I       0x3C4	/* Sequencer Index */
#define VGA_PEL_IW      0x3C8	/* PEL Write Index */
#define VGA_PEL_IR      0x3C7	/* PEL Read Index */

/* standard VGA indexes max counts */
#define VGA_CRT_C       0x19	/* Number of CRT Controller Registers */
#define VGA_ATT_C       0x15	/* Number of Attribute Controller Registers */
#define VGA_GFX_C       0x09	/* Number of Graphics Controller Registers */
#define VGA_SEQ_C       0x05	/* Number of Sequencer Registers */
#define VGA_MIS_C       0x01	/* Number of Misc Output Register */

/* Vga colors */
#define	BLACK			0
#define	BLUE			1
#define	GREEN			2
#define	CYAN			3
#define	RED			4
#define	MAGENTA			5
#define	BROWN			6
#define	LIGHTGRAY		7
#define	DARKGRAY		8
#define	LIGHTBLUE		9
#define	LIGHTGREEN		10
#define	LIGHTCYAN		11
#define	LIGHTRED		12
#define	LIGHTMAGENTA		13
#define	YELLOW			14
#define	WHITE			15

class VGA:public Tinterface {
private:
  u16_t * fb;
  off_t offs;
  size_t bufsize;

  struct GEOM {
    size_t height;
    size_t width;
  } geom;

  void scroll_up();
  void OutRaw(u16_t ch);

public:
  VGA();

  size_t write(off_t offset, const void *buf, size_t count);

  void reset();

  void MoveCursorXY(u8_t x, u8_t y);
  void MoveCursor(off_t offset);
  void SetCursorType(unsigned char show);

  void SetFont(unsigned char *fnt);
};

#define _NORMALCURSOR	1
#define _NOCURSOR	0

#endif
