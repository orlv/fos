/*
  kernel/include/atomic.h
  Взято из linux-2.6.17/include/asm-i386/atomic.h
 */

#ifndef _CPP_ATOMIC
#define _CPP_ATOMIC

#include <asm/xchg.h>

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */

class atomic_t {
private:
  volatile int __value;
public:
  atomic_t()
    {
      __value = 0;
    }

  inline int value()
  {
    return __value;
  }

  inline int set(int i)
  {
    return (__value = i);
  }

  /**
   * add - add integer to atomic variable
   * @i: integer __value to add
   * 
   * Atomically adds @i to "count".
   */
  inline void add(int i)
  {
    __asm__ __volatile__("addl %1,%0"
			 :"=m" (__value)
			 :"ir" (i), "m" (__value));
  }

  /**
   * sub - subtract the atomic variable
   * @i: integer value to subtract
   * 
   * Atomically subtracts @i from "count".
   */
  inline void sub(int i)
  {
    __asm__ __volatile__("subl %1,%0"
			 :"=m" (__value)
			 :"ir" (i), "m" (__value));
  }

  /**
   * sub_and_test - subtract value from variable and test result
   * @i: integer value to subtract
   * 
   * Atomically subtracts @i to "count" and returns
   * true if the result is zero, or false for all
   * other cases.
   */
  inline int sub_and_test(int i)
  {
    unsigned char c;

    __asm__ __volatile__("subl %2,%0; sete %1"
			 :"=m" (__value), "=qm" (c)
			 :"ir" (i), "m" (__value) : "memory");
    return c;
  }

  /**
   * inc - increment atomic variable
   * 
   * Atomically increments "count" by 1.
   */ 
  inline void inc()
  {
    __asm__ __volatile__("incl %0"
			 :"=m" (__value)
			 :"m" (__value));
  }

  /**
   * dec - decrement atomic variable
   * 
   * Atomically decrements "count" by 1.
   */ 
  inline void dec()
  {
    __asm__ __volatile__("decl %0"
			 :"=m" (__value)
			 :"m" (__value));
  }

  /**
   * dec_and_test - decrement and test
   * 
   * Atomically decrements "count" by 1 and
   * returns true if the result is 0, or false for all other
   * cases.
   */ 
  inline int dec_and_test()
  {
    unsigned char c;
    __asm__ __volatile__("decl %0; sete %1"
			 :"=m" (__value), "=qm" (c)
			 :"m" (__value) : "memory");
    return c != 0;
  }

  /**
   * inc_and_test - increment and test 
   * 
   * Atomically increments "count" by 1
   * and returns true if the result is zero, or false for all
   * other cases.
   */ 
  inline int inc_and_test()
  {
    unsigned char c;
    __asm__ __volatile__("incl %0; sete %1"
			 :"=m" (__value), "=qm" (c)
			 :"m" (__value) : "memory");
    return c != 0;
  }

  /**
   * add_negative - add and test if negative
   * @i: integer value to add
   * 
   * Atomically adds @i to "count" and returns true
   * if the result is negative, or false when
   * result is greater than or equal to zero.
   */ 
  inline int add_negative(int i)
  {
    unsigned char c;
    __asm__ __volatile__("addl %2,%0; sets %1"
			 :"=m" (__value), "=qm" (c)
			 :"ir" (i), "m" (__value) : "memory");
    return c;
  }

  /**
   * add_return - add and return
   * @i: integer value to add
   *
   * Atomically adds @i to "count" and returns @i + "count"
   */
  inline int add_return(int i)
  {
    int __i;
#ifdef CONFIG_M386
    unsigned long flags;
    if(unlikely(boot_cpu_data.x86==3))
      goto no_xadd;
#endif
    /* Modern 486+ processor */
    __i = i;
    __asm__ __volatile__("xaddl %0, %1;"
			 :"=r"(i)
			 :"m"(__value), "0"(i));
    return i + __i;

#ifdef CONFIG_M386
  no_xadd: /* Legacy 386 processor */
    //local_irq_save(flags);
    __i = this->__value(v);
    this->set(v, i + __i);
    //local_irq_restore(flags);
    return i + __i;
#endif
  }

  inline int sub_return(int i)
  {
    return this->add_return(-i);
  }

  inline int cmpxchg(int old, int _new)
  {
    return _cmpxchg(&__value, old, _new);
  }

  inline int _xchg(int _new)
  {
    return xchg(&__value, _new);
  }

  /**
   * add_unless - add unless the number is a given value
   * @a: the amount to add to "count"...
   * @u: ...unless "count" is equal to u.
   *
   * Atomically adds @a to "count", so long as it was not @u.
   * Returns non-zero if "count" was not @u, and zero otherwise.
   */
  inline bool add_unless(int a, int u)
  {
    int c, old;
    c = this->value();
    for (;;) {
      if (c == u)
	break;
      old = this->cmpxchg(c, c+a);
      if (old == c)
	break;
      c = old;
    }
    return c != u;
  }
  
  inline int inc_not_zero()
  {
    return add_unless(1, 0);
  }

  inline int inc_return()
  {
    return add_return(1);
  }
  
  inline int dec_return()
  {
    return sub_return(1);
  }
};

#endif
