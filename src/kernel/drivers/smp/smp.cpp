/*
 * Copyright (C) 2008 Sergey Gridassov
 */
#include <fos/printk.h>
#include <fos/system.h>
#include "smp.h"

int SMP::mpf_checksum(u8_t *mp, int len) {
	int sum = 0;

	while(len--)
		sum += *mp++;

	return sum & 0xFF;
}

bool SMP::ScanConfig(u32_t base, u32_t length) {
	u32_t *bp = (u32_t *)base;
	struct intel_mp_floating *mpf;

	printk("Scan SMP from %p for %ld bytes.\n", bp,length);
	if (sizeof(*mpf) != 16)
		printk("Error: MPF size\n");

	while (length > 0) {
		mpf = (struct intel_mp_floating *)bp;
		if ((*bp == SMP_MAGIC_IDENT) &&
			(mpf->mpf_length == 1) &&
			!mpf_checksum((unsigned char *)bp, 16) &&
			((mpf->mpf_specification == 1)
				|| (mpf->mpf_specification == 4)) {

			found_config = 1;
			printk("found SMP MP-table at %08lx\n",
						mpf);
			return true;
                 }
                 bp += 4;
                 length -= 16;
         }
	return false;
}


SMP::SMP() {
	found_config = false;

	if(ScanConfig(0x0, 0x400) || ScanConfig(639 * 0x400, 0x400) || ScanConfig(0xF0000, 0x10000)) 
		printk("SMP detected\n");
	else {
		u16_t address = *(u16_t *)0x40E;
		address <<=4;
		if(address) {
			if(ScanConfig(address, 0x400)) 
				printk("SMP detected\n");
		} else
			printk("No EBDA\n");
		printk("SMP not detected\n");
	}

}
