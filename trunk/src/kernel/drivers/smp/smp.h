/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef SMP_H
#define SMP_H

#include "smp_defs.h"

class SMP {
	bool ScanConfig(u32_t base, u32_t len);
	int mpf_checksum(u8_t *mp, int len);
public:
	SMP();

	bool found_config;
};
#endif

