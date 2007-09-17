#define _FOS_MYTID             10  /* позволяет потоку узнать свой Thread ID */

#define u32_t unsigned int

static inline u32_t sys_call(volatile u32_t cmd, volatile u32_t arg) {
  u32_t result;
  __asm__ __volatile__ ("int $0x30":"=a"(result):"b"(cmd), "c"(arg));
  return result;
}

int my_tid() {
#if !FOS
  return 0;
#else
  return sys_call(_FOS_MYTID, 0);
#endif
}
