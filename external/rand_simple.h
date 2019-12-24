#pragma once
#ifndef ONCE_INC_RAND_SIMPLE_H_
#define ONCE_INC_RAND_SIMPLE_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void rand_simple_set_seed_auto(void);
void rand_simple_set_seed(const uint32_t sseed);
uint32_t rand_simple(void);
uint32_t rand_simple_parkmiller(void);
void randombytes(uint8_t* mem, const size_t count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // ONCE_INC_RAND_SIMPLE_H_
