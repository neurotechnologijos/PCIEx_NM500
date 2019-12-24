/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_NTAPCIE_INT_H_
#define ONCE_INC_NTAPCIE_INT_H_

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#include "ntia_api_data_types.h"
#include "ntia_api_data_types_ll.h"

#include "crc32.h"

#ifdef __amd64__
#include <x86intrin.h>
#endif // __amd64__

#if defined(__GNUC__) || defined(__CLANG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

static inline uint64_t _cpu_get_tick_count(void)
{
#ifdef __amd64__
  return _rdtsc();
#else
  return 0;
#endif // __amd64__
}

static inline uint32_t dev_handle_hash_calc(const struct nta_dev_handle_t* const dev_handle)
{
  if (dev_handle == NULL)
  {
    return 0xFFFFFFFFul;
  }

  return crc32_mem_value(dev_handle, sizeof(dev_handle->_iox_handle));
}

static inline bool dev_handle_is_valid(const struct nta_dev_handle_t* const dev_handle)
{
  bool result = false;
  if (dev_handle == NULL || dev_handle->_iox_handle == NULL)
  {
    return false;
  }

  result = (dev_handle->_signature == dev_handle_hash_calc(dev_handle));
  return result;
}

static inline void dev_handle_hash_update(struct nta_dev_handle_t* const dev_handle)
{
  if (dev_handle == NULL)
  {
    return;
  }
  else
  {
    dev_handle->_signature = dev_handle_hash_calc(dev_handle);
  }
}

static inline void dev_handle_invalidate(struct nta_dev_handle_t* const dev_handle)
{
  if (dev_handle == NULL)
  {
    return;
  }
  else
  {
    memset(dev_handle, 0, sizeof(*dev_handle));
    dev_handle->_signature = 0xFFFFFFFFul;
  }
}

static inline void devs_list_clear(struct nta_pcidev_list_t * const devs_list)
{
  memset(devs_list, 0, sizeof(*devs_list));
}

#if defined(__GNUC__) || defined(__CLANG__)
#pragma GCC diagnostic pop
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

enum ntpcie_nn_error_t ntpcie_card_reset(struct nta_dev_handle_t* const dev_handle);
enum ntpcie_nn_error_t ntpcie_card_wait_ready(const struct nta_dev_handle_t* const dev_handle,
                                              size_t wait_cycles,
                                              union pcie_card_status_t* const _status);
enum ntpcie_nn_error_t ntpcie_card_wait_ready_data(const struct nta_dev_handle_t* const dev_handle,
                                                   size_t wait_cycles,
                                                   union pcie_card_status_t* const _status);
void nn_state_reset(struct nn_state_t* const _state);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ONCE_INC_NTAPCIE_INT_H_
