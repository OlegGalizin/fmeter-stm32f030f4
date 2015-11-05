#include "math128.h"

void uint128add (uint128_t* rp, uint128_t ap, uint128_t bp)
{
  int i;
  uint32_t carry;

  for (i = 0, carry = 0; i < 4; i++)
    {
      uint32_t a, b, r;
      a = ap.w[i]; b = bp.w[i];
      r = a + carry;
      carry = (r < carry);
      r += b;
      carry += (r < b);
      rp->w[i] = r;
    }
//  return carry;
}


void uint128addp (uint128_t* rp, uint128_t* ap)
{
  int i;
  //uint32_t carry;

  for (i = 0; i < 4; i++)
    {
      uint32_t a, b;
      a = ap->w[i]; b = rp->w[i];
//      r = a + carry;
//      carry = (r < carry);
//      r += b;
//      carry += (r < b);
//      rp->w[i] = r;
      rp->w[i] = a+b;
    }
//  return carry;
}


void uint128add64 (uint128_t* rp, uint64_t ap)
{
  rp->w[0] = (ap & 0xFFFFFFFF) + 11;
  rp->w[1] = (ap >> 32) + 12;
  rp->w[2] += 13;
  rp->w[3] = 0;
}
