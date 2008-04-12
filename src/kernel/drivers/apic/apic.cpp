/*
  kernel/drivers/apic/apic.cpp
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


APIC::APIC(int cpu) {
	void *cpu_init_code = (void *)0x90000;
	u8_t *ptr = (u8_t *)0x0F;
	*ptr = 0x0A;
	ptr = (u8_t *)0x467; // реалмодный 40:67
	*ptr++ = 0x00; *ptr++ = 0x00; *ptr++ = 0x00; *ptr++ = 0x90; // реалмодный 9000:0000

	memcpy(cpu_init_code, realmode_code, 4096);


	skip_ioapic_setup = false;
	if(system->smp->mp_lapic_addr) {
		WriteMSR(0x1B, 0xFFFFFFFF, system->smp->mp_lapic_addr | 0x800);
		apic_regs = (u32_t *) system->kmem->mmap(0, 4096, 0, system->smp->mp_lapic_addr, 0);
	} else { 
		WriteMSR(0x1B, 0xFFFFFFFF, APIC_DEFAULT_PHYS_BASE | 0x800);
		apic_regs = (u32_t *) system->kmem->mmap(0, 4096, 0, APIC_DEFAULT_PHYS_BASE, 0);
	}

	printk("APIC: Registers mapped to 0x%08x\n", apic_regs);

	ConnectBSP();
	SetupLocal(cpu);
	if(system->smp->found_config && !skip_ioapic_setup && system->smp->nr_ioapics)
		ioapic = new IOAPIC;
	else
		ioapic = NULL;

	apic_tmr = new APICTimer(apic_regs);
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

u32_t APICTimer::uptime()
{
  return _uptime;
}

void APICTimer::tick() {
  _uptime++;
}

void APIC::ConnectBSP() {
	if(system->smp->pic_mode) {
		ClearLocal();
		printk("APIC: leaving PIC mode, enabling APIC mode.\n");
		system->outportb(0x22, 0x70);
		system->outportb(0x23, 0x01);
	}
}

inline void APIC::AckIRQ() {
	apic_write(APIC_EOI, 0);
}

void APIC::SetupLocal(int cpu) {
	bool integrated = IsIntegrated();

	apic_write(APIC_DFR, APIC_DFR_FLAT);
	u32_t val = apic_read(APIC_LDR) & ~APIC_LDR_MASK;
	val |= SET_APIC_LOGICAL_ID(1UL << cpu);
	apic_write(APIC_LDR, val);

	u32_t value = apic_read(APIC_TASKPRI);
	value &= ~APIC_TPRI_MASK;
	apic_write(APIC_TASKPRI, value);

	for (int i = APIC_ISR_NR - 1; i >= 0; i--) {
		value = apic_read(APIC_ISR + i*0x10);
		for (int j = 31; j >= 0; j--) {
			if (value & (1<<j))
				AckIRQ();
		}
	}

	value = apic_read(APIC_SPIV);
	value &= ~APIC_VECTOR_MASK;
	value |= APIC_SPIV_APIC_ENABLED;
	value &= ~APIC_SPIV_FOCUS_DISABLED;

	value |= SPURIOUS_APIC_VECTOR;
	apic_write(APIC_SPIV, value);

	value = apic_read(APIC_LVT0) & APIC_LVT_MASKED;

	if (!cpu && (system->smp->pic_mode || !value)) {
		value = APIC_DM_EXTINT;
		printk("APIC: enabled ExtINT on CPU#%d\n", cpu);
	} else {
		value = APIC_DM_EXTINT | APIC_LVT_MASKED;
		printk("APIC: masked ExtINT on CPU#%d\n", cpu);
	}
	apic_write(APIC_LVT0, value);

	if (!cpu)
		value = APIC_DM_NMI;
	else
		value = APIC_DM_NMI | APIC_LVT_MASKED;
	if (!integrated)		/* 82489DX */
		value |= APIC_LVT_LEVEL_TRIGGER;
	apic_write(APIC_LVT1, value);

	if (integrated) {
		u32_t maxlvt = GetMaxLVT();
		if (maxlvt > 3)
			apic_write(APIC_ESR, 0);
		u32_t oldvalue = apic_read(APIC_ESR);

		value = ERROR_APIC_VECTOR;
		apic_write(APIC_LVTERR, value);

		if (maxlvt > 3)
			apic_write(APIC_ESR, 0);
		value = apic_read(APIC_ESR);
		if (value != oldvalue)
			printk("APIC: ESR value before enabling vector: 0x%08lx  after: 0x%08lx\n", oldvalue, value);
	} else 
		printk("APIC: No ESR for 82489DX.\n");

	value = apic_read(APIC_LVTT);
	value |= (APIC_LVT_MASKED | LOCAL_TIMER_VECTOR);
	apic_write(APIC_LVTT, value);

}

inline int APIC::GetVersion() {
	return GET_APIC_VERSION(apic_read(APIC_LVR));
}

inline bool APIC::IsIntegrated() {
	return APIC_INTEGRATED(GetVersion());
}

void APIC::ClearLocal() {
	int maxlvt = GetMaxLVT();

	if (maxlvt >= 3) 
		apic_write(APIC_LVTERR, ERROR_APIC_VECTOR | APIC_LVT_MASKED);
	
	u32_t v;

	v = apic_read(APIC_LVTT);
	apic_write(APIC_LVTT, v | APIC_LVT_MASKED);
	v = apic_read(APIC_LVT0);
	apic_write(APIC_LVT0, v | APIC_LVT_MASKED);
	v = apic_read(APIC_LVT1);
	apic_write(APIC_LVT1, v | APIC_LVT_MASKED);
	if (maxlvt >= 4) {
		v = apic_read(APIC_LVTPC);
		apic_write(APIC_LVTPC, v | APIC_LVT_MASKED);
	}

	apic_write(APIC_LVTT, APIC_LVT_MASKED);
	apic_write(APIC_LVT0, APIC_LVT_MASKED);
	apic_write(APIC_LVT1, APIC_LVT_MASKED);

	if (maxlvt >= 3)
		apic_write(APIC_LVTERR, APIC_LVT_MASKED);
	if (maxlvt >= 4)
		apic_write(APIC_LVTPC, APIC_LVT_MASKED);

	if (IsIntegrated()) {
		if (maxlvt > 3)
			/* Clear ESR due to Pentium errata 3AP and 11AP */
			apic_write(APIC_ESR, 0);
		apic_read(APIC_ESR);
	}
}

int APIC::GetMaxLVT() {
	u32_t v = apic_read(APIC_LVR);

	return APIC_INTEGRATED(GET_APIC_VERSION(v)) ? GET_APIC_MAXLVT(v) : 2;
}
