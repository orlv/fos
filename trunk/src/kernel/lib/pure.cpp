
#include <types.h>
#include <fos/printk.h>

asmlinkage void __pure_virtual()
{
  printk("pure virtual function called\n");
  while(1);
}

asmlinkage void __cxa_pure_virtual()
{
  printk("pure virtual function called\n");
  while(1);
}
