#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "tss.h"
#include "gdt.h"

#define BASE_TSK_SEL 0x38
#define SEL_N(SEL) ((SEL)/8)

struct context_t {
  struct TSS *tss;
  gdt_entry descr;
  off_t stack_pl0;
};

void set_initial_task(context_t *context);
void switch_context(context_t *current, context_t *next);
void setup_context(context_t *context,
		   u32_t *pagedir,
		   off_t eip,
		   void *kernel_stack,
		   void *user_stack,
		   u16_t code_segment,
		   u16_t data_segment);
int sched_ready();


/* ------ */

static inline u16_t str()
{
  u16_t tr;
  __asm__ __volatile__ ("str %0":"=a" (tr));
  return tr;
}

static inline u16_t curPID()
{
  return (str() - BASE_TSK_SEL) / 0x08;
}

#endif

