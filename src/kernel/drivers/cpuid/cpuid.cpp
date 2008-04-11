/*
 * drivers/cpuid/cpuid.cpp
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <fos/system.h>
#include <fos/printk.h>
#include <string.h>

bool CPUID::HaveCPUID()
{
  u32_t new_flags, old_flags;
  __asm__ __volatile__(	"pushf\n"
			"popl %%eax\n"
			"movl %%eax, %%ebx\n"
			"xorl $0x00200000, %%eax\n"
			"pushl %%eax\n"
			"popf\n"
			"pushf\n"
			"popl %%eax\n":"=b"(old_flags), "=a"(new_flags));
  return old_flags != new_flags;
}

void CPUID::GetVendorAndMax(char *buf)
{
  __asm__ __volatile__("cpuid" :
		       "=a" (max_std), 
		       "=b" (*((u32_t *)buf)),
		       "=d" (*((u32_t *)buf + 1)),
		       "=c" (*((u32_t *)buf + 2)):
		       "a" (0));
  buf[12] = 0;

  __asm__ __volatile__("cpuid" : "=a"(max_ext): "a"(0x80000000):"ebx", "ecx", "edx");

  if(max_ext <= 0x80000000)
    max_ext = 0;

}

CPUID::CPUID()
{
  if(!HaveCPUID()) {
    printk("CPUID: 386 or 486 detected\n");
    features_ecx = 0;
    features_edx = 0;
    return;
  }

  char buf[13];
  GetVendorAndMax(buf);

  if(strcmp(buf, "GenuineIntel") == 0) 
    vendor_code = VENDOR_INTEL;
  else if(strcmp(buf, "AuthenticAMD") == 0)
    vendor_code = VENDOR_AMD;
  else {
    vendor_code = VENDOR_UNKNOWN;
    printk("CPUID: warning: unknown vendor\n");
  }

  model = 0;
  family = 0;
  stepping = 0;

  for(u32_t i = 1; i <= max_std; i++) {
    u32_t eax, ebx, ecx, edx;
    __asm__ __volatile__ ("cpuid":"=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx):"a"(i));
    switch(i) {
    case 1:
      stepping = eax & 0x0F;
      model = ((eax & 0xF0) >> 4) | ((eax & 0xF0000) >> 12);
      family = ((eax & 0xF00) >> 8) | ((eax & 0xFF00000) >> 16);
      /* парсинг ebx */
      features_ecx = ecx;
      features_edx = edx;
      break;
      /*  2-0x0D */
    }
  }
  /* extended */

  printk("CPUID: Vendor: %s -- code %d, family 0x%X model 0x%X stepping 0x%X\n", buf, vendor_code, family, model, stepping);
}
