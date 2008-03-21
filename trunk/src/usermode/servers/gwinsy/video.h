/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef VIDEO_H
#define VIDEO_H
int video_init(const char *driver, int width, int height, int bpp, context_t **screen, jump_table_t **jmptbl);

extern jump_table_t *jmptbl;
extern context_t *screen;

#define Blit(from,to,x,y,w,h,src_x,src_y) (jmptbl->Blit)(from,to,x,y,w,h,src_x,src_y)
#define SetPixel(x,y,RGB,context) (jmptbl->SetPixel)(x,y,RGB,context)
#define GetPixel(x,y,context) (jmptbl->GetPixel)(x,y,context)
#define Rect(x,y,w,h,RGB,context) (jmptbl->Rect)(x,y,w,h,RGB,context)
#endif
