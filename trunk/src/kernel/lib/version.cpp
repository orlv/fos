/*
    kernel/main/version.cpp
    Copyright (C) 2005 Oleg Fedorov
*/

#include <types.h>
#include <fos/version.h>

const char *version = VERSION;
u32_t build = BUILD;
const char *compile_date = __DATE__;
const char *compile_time = __TIME__;
