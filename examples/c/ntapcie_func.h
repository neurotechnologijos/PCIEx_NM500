/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_NTAPCIE_FUNC_H_
#define ONCE_INC_NTAPCIE_FUNC_H_

#include <stdbool.h>

#include "ntia_api_data_types.h"

#include "crc32.h"

// force set MAX neurons count for PCIe NN card (hardware deps) - only for examples
#define MAX_NEURONS_COUNT (16384)
#define MAX_KBS_SLOTS     (2)

struct test_data_t
{
  uint16_t context;
  uint16_t category;
  nn_vector_comp_t comps[NN_NEURON_COMPONENTS];
};

#pragma pack(push)
#pragma pack(1)

// sample of KB
struct nn_sample_kb_t
{
  uint64_t id;
  size_t neurons_count;
  struct nn_neuron_t neurons_state[MAX_NEURONS_COUNT];
  uint32_t crc32; // ATT: must be last element
};

#pragma pack(pop)

inline static bool nn_sample_kb_is_valid(const struct nn_sample_kb_t* const kb)
{
  bool result = false;
  if (kb == NULL)
  {
    return false;
  }

  result = (kb->crc32 == crc32_mem_value(kb, sizeof(*kb) - sizeof(kb->crc32)));
  return result;
}

inline static void nn_sample_kb_crc32_update(struct nn_sample_kb_t* const kb)
{
  kb->crc32 = crc32_mem_value(kb, sizeof(*kb) - sizeof(kb->crc32));
}

void card_reset(struct nta_dev_handle_t* const dev_handle);
void card_reset_stress_test(struct nta_dev_handle_t* const dev_handle);
void card_view_info(struct nta_dev_handle_t* const dev_handle);
void nntest_full_test(struct nta_dev_handle_t* const dev_handle);
void nntest_simple_test(struct nta_dev_handle_t* const dev_handle);
void nntest__register_read(struct nta_dev_handle_t* const dev_handle);
void nntest_register_write(struct nta_dev_handle_t* const dev_handle);
void nntest_neuron_dump(struct nta_dev_handle_t* const dev_handle);
void nntest_forget_all(struct nta_dev_handle_t* const dev_handle);
void nntest_kb_store(struct nta_dev_handle_t* const dev_handle);
void nntest_kb_load(struct nta_dev_handle_t* const dev_handle);
void nntest_kbs_compare(struct nta_dev_handle_t* const dev_handle);

#endif // ONCE_INC_NTAPCIE_FUNC_H_
