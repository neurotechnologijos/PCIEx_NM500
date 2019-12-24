#pragma once
#ifndef ONCE_INC_HASH32DJB2_H_
#define ONCE_INC_HASH32DJB2_H_

#include <stdint.h>
#include <stddef.h>

// *INDENT-OFF*
// clang-format off

static inline
uint32_t __hash32djb2_shl(const void* mem, size_t count)
{
  uint32_t hash          = 5381u;
  const uint8_t* pointer = (const uint8_t*)mem;
  while (count--)
  {
    hash = ((hash << 5) + hash) + *pointer++;
  }
  return hash;
}

#ifdef __arm__
static inline
uint32_t __hash32djb2_mul(const void* mem, size_t count)
{
  uint32_t hash          = 5381u;
  const uint8_t* pointer = (const uint8_t*)mem;
  while (count--)
  {
    //  hash = (hash * 33) + *pointer++;
    __asm__ __volatile__ ("mul %0, %1, %2" : "=r"(hash) : "r"(hash), "r"(33u));
    hash += (*pointer++);
  }
  return hash;
}

__attribute__((__always_inline__))
static inline
uint32_t __hash32djb2_mac(const void* mem, size_t count)
{
  uint32_t hash          = 5381u;
  const uint8_t* pointer = (const uint8_t*)mem;
  while (count--)
  {
    __asm__ __volatile__ ("mla %0, %1, %2, %3" : "=r"(hash) : "r"(hash), "r"(33u), "r"(*pointer++));
  }
  return hash;
}

__attribute__((__always_inline__))
static inline
uint32_t __hash32djb2_mac_string(char const *string_pointer)
{
  uint32_t hash          = 5381u;
  while (*string_pointer)
  {
    __asm__ __volatile__ ("mla %0, %1, %2, %3" : "=r"(hash) : "r"(hash), "r"(33u), "r"(*string_pointer++));
  }
  return hash;
}

#else // NOT __arm__

static inline
uint32_t __hash32djb2_mul(const void* mem, size_t count)
{
  uint32_t hash          = 5381u;
  const uint8_t* pointer = (const uint8_t*)mem;
  while (count--)
  {
    hash = (hash * 33) + *pointer++;
  }
  return hash;
}

#endif // __arm__

// *INDENT-ON*
// clang-format on

#endif  // ONCE_INC_HASH32DJB2_H_
