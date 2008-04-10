/*
  kernel/drivers/apic.cpp
  Copyright (C) 2008 Sergey Gridassov
*/

#include "apic.h"
#include <fos/printk.h>
#include <sys/msr.h>
#include <string.h>
#include <fos/system.h>
extern "C" {
	extern char realmode_code[];
}
APIC::APIC() {

	u32_t apic_high;
	u32_t apic_low;

	ReadMSR(IA32_APIC_BASE, &apic_low, &apic_high);
	u32_t addr = apic_low & APIC_MSR_BASE_M;

	printk("APIC: Memory at: 0x%08X\n", addr);


	apic_regs = (u32_t *)system->kmem->mmap(0, 4096, 0, 0xFEE00000, 0);

	printk("APIC mapped to %x\n", apic_regs);
/*
	printk("APIC: ID: 0x%08X\n", apic[APIC_DWREG_ID]);
	printk("APIC: Version: 0x%08X\n", apic[APIC_DWREG_VER]);

	// запускаем SMP
	apic[APIC_DWREG_SVR] = (apic[APIC_DWREG_SVR] & 0xffffff0f) | 0x100;
	apic[APIC_DWREG_LVT_ERR] = (apic[APIC_DWREG_LVT_ERR] & 0xffffff0f) | 0x20;

	void *cpu_init_code = system->kmem->mmap(0, 4096, 0, 0x90000, 0);

	u8_t *ptr = (u8_t *)0x0F;
	*ptr = 0x0A;
	ptr = (u8_t *)0x467; // реалмодный 40:67
	*ptr++ = 0x00; *ptr++ = 0x00; *ptr++ = 0x00; *ptr++ = 0x90; // реалмодный 9000:0000

	memcpy(cpu_init_code, realmode_code, 4096);

	printk("APIC: INIT ");
	apic[APIC_DWREG_ICR0] = 0x000C0500;
	for (int i=0; i<0x1000000; i++) __asm__ __volatile__("nop\nnop"); // delay 10 ms
	printk("SIPI ");
	apic[APIC_DWREG_ICR0] = 0x000C0690;
	for (int i=0; i<0x1000000; i++) __asm__ __volatile__("nop"); // delay 200 ms
	printk("SIPI ");
	apic[APIC_DWREG_ICR0] = 0x000C0690;
	for (int i=0; i<0x1000000; i++) __asm__ __volatile__("nop"); // delay 200 ms

	printk("OK\n");

	apic[APIC_DWREG_SVR] = (apic[APIC_DWREG_SVR] & 0xffffff0f) | 0x100;
	apic[APIC_DWREG_LVT_LINT0] = (apic[APIC_DWREG_LVT_LINT0] & 0xfffe00ff) | 0x5700;
	apic[APIC_DWREG_LVT_LINT1] = (apic[APIC_DWREG_LVT_LINT1] & 0xfffe00ff) | 0x5700;
	printk("APIC: Virtual wire mode set\n");
*/
#if 0
	printk("APIC: Disabling (for more safety)\n");
	// и выключаем APIC к черту
	ReadMSR(IA32_APIC_BASE, &apic_low, &apic_high);

	printk("APIC: MSR: 0x%08X%08X\n", apic_high, apic_low);

	apic_low &= ~(1 << 11);
	WriteMSR(IA32_APIC_BASE, apic_low, apic_high);

	ReadMSR(IA32_APIC_BASE, &apic_low, &apic_high);

	printk("APIC: MSR: 0x%08X%08X\n", apic_high, apic_low);

	printk("APIC: disabled\n");
#endif

}

void APIC::mask(int n) {
	system->panic("APIC: mask(%d) not implemented\n", n);
}

void APIC::unmask(int n) {
	system->panic("APIC: unmask(%d) not implemented\n", n);
}

void APIC::lock() {
	system->panic("APIC: lock() not implemented\n");
}

void APIC::unlock() {
	system->panic("APIC: unlock() not implemented\n");
}

void APIC::Route(int n) {
	system->panic("APIC: Route(%d) not implemented\n", n);
}

void APIC::EOI(int n) {
	system->panic("APIC: EOI(%d) not implemented\n", n);
}

void APIC::setHandler(int n, void *handler) {
	system->panic("APIC: setHandler(%d, 0x%X) not implemented\n", n, handler);
}

void *APIC::getHandler(int n) {
	system->panic("APIC: getHandler(%d) not implemented\n", n);
	return NULL;
}

Timer *APIC::getTimer() {
	return apic_tmr;
}
