/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_NTIA_API_DATA_TYPES_H_
#define ONCE_INC_NTIA_API_DATA_TYPES_H_

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "sorry, tested only for LITTLE_ENDIAN"
#endif // __BYTE_ORDER__

#ifdef __cplusplus
  #include <cassert>
  #include <cstdint>
  #include <type_traits>
#else
  #include <assert.h>
  #include <stdbool.h>
  #include <stdint.h>
  #include <stddef.h>
#endif // __cplusplus

// NN vector components data type (uint16_t soon?)
typedef uint8_t nn_vector_comp_t;

// *INDENT-OFF*
// clang-format off

//--------------- TODO: move to appropriate place
// general PCIe dev parameters
#ifndef NTIA_PCIE_MAX_CARDS
#define NTIA_PCIE_MAX_CARDS         (16)

struct nta_pcidev_info_t
{
  uint16_t             bus;
  uint16_t             slot;
  uint16_t             func;
};

struct nta_pcidev_list_t
{
  uint16_t                 pci_id_vendor;
  uint16_t                 pci_id_device;
  size_t                   devs_count;
  struct nta_pcidev_info_t devices[NTIA_PCIE_MAX_CARDS];
};
#endif // NTIA_PCIE_MAX_CARDS
//-----------------------------------------------


// general NN parameters
#define NN_NEURON_COMPONENTS        (256)
#define NN_MAX_RESP_COUNT           (85)
// influence fields default values
#define NN_DEF_MAXIF                (0x4000)
#define NN_DEF_MINIF                (0x0002)

enum ntpcie_nn_error_t
{
  NTPCIE_ERROR_SUCCESS               = 0x00000000L,
  NTPCIE_ERROR_UNKNOWN,
  NTPCIE_ERROR_INVALID_HANDLE,
  NTPCIE_ERROR_WAIT_TIMEOUT,
  NTPCIE_ERROR_IO,
  NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH,

  NTPCIE_ERROR_CARD_RESET,
  NTPCIE_ERROR_CARD_OPEN,
  NTPCIE_ERROR_CARD_FAULT,

  NTPCIE_ERROR_DATA_READ,
  NTPCIE_ERROR_DATA_WRITE,
  NTPCIE_ERROR_REG_READ,
  NTPCIE_ERROR_REG_WRITE,
  NTPCIE_ERROR_SERV_READ,
  NTPCIE_ERROR_SERV_WRITE,
  NTPCIE_ERROR_NO_DATA_FOR_READ,

  NTPCIE_ERROR_ARGS_NULL_POINTER,
  NTPCIE_ERROR_ARGS_COMPS_COUNT,
  NTPCIE_ERROR_ARGS_DIST_EVAL,
  NTPCIE_ERROR_ARGS_CLASSIFIER,
  NTPCIE_ERROR_ARGS_CONTEXT,
  NTPCIE_ERROR_ARGS_CATEGORY,
  NTPCIE_ERROR_ARGS_INFLUENCE_FIELDS,
  NTPCIE_ERROR_ARGS_RESP_COUNT,

  NTPCIE_ERROR_KBASE_EOF,

  NTPCIE_ERROR_ITEMS_COUNT    // MAX value for ERROR codes
};

enum nn_classifier_t
{
  NN_CLASSIFIER_RBF = 0x00u,
  NN_CLASSIFIER_KNN = 0x01u,
};

enum nn_dist_eval_t
{
  NN_DIST_EVAL_L1   = 0x00u,
  NN_DIST_EVAL_LSUP = 0x01u,
};

struct nn_state_t
{
  size_t               neurons_overall;          ///< overall neurons count on NN
  size_t               neurons_committed;        ///< committed neurons count
  uint64_t             kbase_id;                 ///< ID of loaded knowledge base
  // simple performance counters
  size_t               count_loop_wait_ready;    ///< loops count for waiting "results ready" of last operation
  size_t               cpu_ticks_last_oper;      ///< took CPU ticks for last operation
  size_t               cpu_ticks_total_learn;    ///< took total CPU ticks for learn operation (since last reset)
  size_t               cpu_ticks_total_class;    ///< took total CPU ticks for classify operation (since last reset)
  size_t               vecs_count_total_learn;   ///< total vectors learning (since last reset)
  size_t               vecs_count_total_class;   ///< total vectors classified (since last reset)
};

struct nta_dev_handle_t
{
  void*                _iox_handle;
  uint32_t             _signature;
  struct nn_state_t    nn_state;
};

// "wire" datatypes for IO: must be exact size and fixed fields order
#pragma pack(push)
#pragma pack(1)

struct response_neuron_state_t
{
  uint16_t             distance;
  uint16_t             category     : 15;
  uint16_t             degenerated  : 1;
  uint16_t             id;
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
    static_assert(std::is_pod<struct response_neuron_state_t>::value,
                  "response_neuron_state_t is not POD");
#endif // __cplusplus
    static_assert((sizeof(struct response_neuron_state_t) == 6),
                  "sizeof(struct response_neuron_state_t) != 6");
// +-------------------------------------------------------------------------------------------+

// neuron structure for save and restore base of knowledge
struct nn_neuron_t
{
  uint8_t            opcode;                     ///< operation
  uint8_t            ncr;                        ///< ncr: context (+ncr: distance eval)
  uint16_t           category;                   ///< value of category
  uint16_t           aif;                        ///< value of aif
  uint16_t           minif;                      ///< value of minif
  nn_vector_comp_t   comp[NN_NEURON_COMPONENTS]; ///< values of neuron components
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
    static_assert(std::is_pod<struct nn_neuron_t>::value, "nn_neuron_t is not POD");
#endif // __cplusplus
    // hardcoded: for component type == uint8_t
    static_assert((sizeof(struct nn_neuron_t) == 264),
                  "sizeof(struct nn_neuron_t) != 264");
// +-------------------------------------------------------------------------------------------+

#pragma pack(pop)

// *INDENT-ON*
// clang-format on

#endif  // ONCE_INC_NTIA_API_DATA_TYPES_H_
