/*
  kernel/drivers/apic/apic.h
  Copyright (C) 2008 Sergey Gridassov
*/

#ifndef __APIC_H
#define __APIC_H

#include <types.h>
#include <fos/drivers/interfaces/interruptcontroller.h>
#include "ioapic.h"
#include "timer.h"
#include "apic_defs.h"

class APIC: public InterruptController  {
	friend class APICTimer;
	friend class IOAPIC;
private:
	IOAPIC *ioapic;
	u32_t * volatile apic_regs;
	APICTimer *apic_tmr;
	void ConnectBSP();
	void SetupLocal(int cpu);
	void ClearLocal();
	int GetMaxLVT();
	bool ModernAPIC();
	inline bool IsIntegrated();
	inline int GetVersion();
	inline void AckIRQ();
	inline void apic_write(u32_t reg, u32_t v) {
		apic_regs[reg] = v;
	}

	inline u32_t apic_read(u32_t reg) {
		return apic_regs[reg];
	}
	bool skip_ioapic_setup;

	void SyncArbIDs();
	void WaitICRIdle();
public:
	APIC(int cpu);
	void mask(int n);
	void unmask(int n);

	void lock();
	void unlock();

	void Route(int n);
	void setHandler(int n, void *handler);
	void *getHandler(int n);

	void EOI(int irq);

	Timer *getTimer();

	int GetPhysicalBroadcast();
};


#endif
