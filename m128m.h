#include <stdint.h>

typedef union
{
  uint32_t w[4];
  uint64_t d[2];
  uint16_t h[8];
} uint128_t;

extern void uint128addpx (uint128_t* dst, uint128_t* ap);
