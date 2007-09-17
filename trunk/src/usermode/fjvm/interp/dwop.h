#ifndef DWOP_H_
#define DWOP_H_

#include "../types.h"

// 64-bit integers.
void long_shl(u4 hi, u4 lo, u1 count, u4 *res_hi, u4 *res_lo);
void long_shr(u4 hi, u4 lo, u1 count, u4 *res_hi, u4 *res_lo);
void long_ushr(u4 hi, u4 lo, u1 count, u4 *res_hi, u4 *res_lo);

#endif /*DWOP_H_*/
