/*
 *         FOS Graphics System
 * Copyright (c) 2008 Sergey Gridassov
 */

#ifndef UNIVERSALBPP_H
#define UNIVERSALBPP_H
extern mode_callbacks *calltable;
#define FlushContext(a,b,c,d,e,f,g,h) (calltable->FlushContext)(a,b,c,d,e,f,g,h)
#define border(a,b,c,d,e) (calltable->border)(a,b,c,d,e)
#define DrawRect(a,b,c,d,e,f) (calltable->DrawRect)(a,b,c,d,e,f)
#define SetPixel(a,b,c,d) (calltable->SetPixel)(a,b,c,d)
#define GetPixel(a,b,c) (calltable->GetPixel)(a,b,c)
#endif
