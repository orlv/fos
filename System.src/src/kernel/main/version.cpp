/*
	kernel/main/version.cpp
	Copyright (C) 2005 Oleg Fedorov
*/

#include <version.h>
#include <types.h>

const char *version = VERSION;
u32_t build = BUILD;
const char *compile_date = __DATE__;
const char *compile_time = __TIME__;
