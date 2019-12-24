#pragma once
#ifndef ONCE_INC_CRC32_H_
#define ONCE_INC_CRC32_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

extern const uint32_t ntia_crc32_table[];

// *INDENT-OFF*
// clang-format off

static inline
void crc32_reset_crc_acc(uint32_t* const crc_acc)
{
  *crc_acc = 0xFFFFFFFFu;
}

static inline
uint_least32_t crc32_get_value(const uint32_t* const crc_acc)
{
  return (*crc_acc ^ 0xFFFFFFFFu);
}

static inline
void crc32_add_single_byte(uint32_t* const crc_acc, const uint8_t single_byte)
{
  *crc_acc = (*crc_acc >> 8) ^ ntia_crc32_table[(*crc_acc ^ single_byte) & 0xFF];
}

static inline
void crc32_add_multi_byte(uint32_t* const crc_acc, const void* const mem, size_t count)
{
  for (const uint8_t* pointer = (const uint8_t*)mem; count--; ++pointer)
  {
    crc32_add_single_byte(crc_acc, *pointer);
  }
}

static inline
uint32_t crc32_mem_value(const void* const mem, size_t count)
{
  uint32_t crc_acc = 0xFFFFFFFFu;
  const uint8_t* pointer = (const uint8_t*)mem;
  while (count--)
  {
    crc_acc = (crc_acc >> 8) ^ ntia_crc32_table[(crc_acc ^ *pointer++) & 0xFF];
  }
  return crc_acc ^ 0xFFFFFFFFu;
}

// *INDENT-ON*
// clang-format on

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // ONCE_INC_CRC32_H_
