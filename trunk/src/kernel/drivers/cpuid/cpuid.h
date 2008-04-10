/*
 * drivers/cpuid/cpuid.h
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef __CPUID_H
#define __CPUID_H

#define VENDOR_UNKNOWN	0
#define VENDOR_INTEL	1
#define VENDOR_AMD	2

#define FEATURE_APIC	(1 << 9)

class CPUID {
private:
	bool HaveCPUID();
	void GetVendorAndMax(char *buf);
	u32_t max_std, max_ext;

public:
	CPUID();
	u32_t features_ecx, features_edx, vendor_code, model, family, stepping;
};
#endif
