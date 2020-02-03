#include "rand_simple.h"

static uint32_t seed_rand_simple = 0x7267FA76UL;


#if defined(__amd64__) || defined(_M_X64)
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif // _MSC_VER

void rand_simple_set_seed_auto(void)
{
  seed_rand_simple = __rdtsc() & 0x00000000FFFFFFFFUL;
}

#endif // __amd64__


void rand_simple_set_seed(const uint32_t sseed)
{
  seed_rand_simple = sseed;
}

uint32_t rand_simple(void)
{
  seed_rand_simple = (((uint64_t)seed_rand_simple) * 1103515245UL + 12345UL) % ((0x00000001UL << 31) - 1);
  return seed_rand_simple;
}

uint32_t rand_simple_parkmiller(void)
{
  seed_rand_simple = (((uint64_t)seed_rand_simple) * 48271UL) % ((0x00000001UL << 31) - 1);
  return seed_rand_simple;
}

void randombytes(uint8_t* mem, const size_t count)
{
  if (count == 0 || mem == NULL)
  {
    return;
  }
  else
  {
    const uint8_t* const end_mem = mem + count;
    while (1)
    {
      uint32_t v = rand_simple();
      for (unsigned int i = 0; i < sizeof(v); ++i)
      {
        *mem = v & 0x000000FF;
        v  >>= 8;
        if (++mem >= end_mem) break;
      }
    }
  }
}
