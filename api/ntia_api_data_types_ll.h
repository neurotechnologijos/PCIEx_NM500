/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_NTIA_API_DATA_TYPES_LL_H_
#define ONCE_INC_NTIA_API_DATA_TYPES_LL_H_

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
#endif // __cplusplus

#include "ntia_api_data_types.h"

// *INDENT-OFF*
// clang-format off

// PCIe address
#define NTPCIE_DEVICE_ADDRESS_DATA      (0x0000ul)
#define NTPCIE_DEVICE_ADDRESS_NET_INFO  (0x0110ul)
#define NTPCIE_DEVICE_ADDRESS_STATUS    (0x0114ul)
#define NTPCIE_DEVICE_ADDRESS_RESET     (0x0118ul)

// IO transfer block size in bytes
#define NTPCIE_DATA_BLOCK_SIZE          (sizeof(uint32_t))

// MAX wait loop counters
#define NTPCIE_MAX_CYCLES_STD           (2500)
#define NTPCIE_MAX_CYCLES_EXT           (5000)

// internal command and data registers in NN
enum nn_int_register_t
{
  /// NN neuron registers addresses
  CM_NCR        = (0x00u), ///< Neuron Context Register
  CM_COMP       = (0x01u), ///< Component
  CM_LCOMP      = (0x02u), ///< Last Component
  CM_INDEXCOMP  = (0x03u), ///< Component index
  CM_DIST       = (0x03u), ///< Distance register
  CM_CAT        = (0x04u), ///< Category register
  CM_AIF        = (0x05u), ///< Active Influence Field
  CM_MINIF      = (0x06u), ///< Minimum Influence Field
  CM_MAXIF      = (0x07u), ///< Maximum Influence Field
  CM_TESTCOMP   = (0x08u), ///< Test Component
  CM_TESTCAT    = (0x09u), ///< Test Category
  CM_NID        = (0x0Au), ///< Neuron Identifier
  CM_GCR        = (0x0Bu), ///< Global Control Register
  CM_RESETCHAIN = (0x0Cu), ///< Points to the first neuron of the chain
  CM_NSR        = (0x0Du), ///< Network Status Register
  CM_POWERSAVE  = (0x0Eu), ///< Dummy register
  CM_FORGET     = (0x0Fu), ///< Clear the (all) neuron’s category register
  CM_NCOUNT     = (0x0Fu), ///< in NR mode - Number of committed neurons; in SR mode - Index of the neuron pointed in the chain;
  CM_FAKE_VALUE = (0xFFu)
};

// code operations
enum  pcie_opcodes_t
{
  NTPCIE_OC_REG_WRITE         =    (0x11u), ///< write value to internal NN register (see enum nn_int_register_t)
  NTPCIE_OC_REG_READ          =    (0x12u), ///< read current value from internmal NN register (see enum nn_int_register_t)
  NTPCIE_OC_VECTOR_LEARN      =    (0x13u), ///< learn NN for vector
  NTPCIE_OC_VECTOR_CLASSIFY   =    (0x14u), ///< classify vector
  NTPCIE_OC_KBASE_STORE       =    (0x16u), ///< transfer KB (internal state of all neuron's) from NN to main RAM
  NTPCIE_OC_KBASE_LOAD        =    (0x17u), ///< transfer KB  (internal state of all neuron's) from RAM to NN
  NTPCIE_OC_NET_RESET         =    (0x1Au), ///< soft reset NN (forget ALL)
  NTPCIE_OC_NEURON_READ       =    (0x1Bu), ///< read internal state of neuron
  NTPCIE_OC_FAULT             =    (0xFEu), ///< PCIe card/NN fault
};

#pragma pack(push)
#pragma pack(1)

union pcie_card_status_t
{
  uint32_t      data;
  struct
  {
    uint32_t    ready           : 1;
    uint32_t    fault           : 1;
    uint32_t    waiting_comps   : 1;
    uint32_t    results_ready   : 1;
    uint32_t    dummy0          : 4;
    uint32_t    result_size     : 7;
    uint32_t    dummy1          : 1;
    uint32_t    readed_count    : 7;
    uint32_t    dummy2          : 1;
    uint32_t    required_count  : 7;
    uint32_t    dummy3          : 1;
  } part;
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
  static_assert(std::is_pod<union pcie_card_status_t>::value,
                "pcie_card_status_t is not POD");
#endif // __cplusplus
  static_assert((sizeof(union pcie_card_status_t) == 4),
                "sizeof(union pcie_card_status_t) != 4");
// +-------------------------------------------------------------------------------------------+

/// learn and recognize pack structures (PCIe exchange)
/// universal pack (header)
struct pcie_data_upack_t
{
  uint8_t     opcode;        ///< operation (enum pcie_opcodes_t)
  uint8_t     reg_address;   ///< address of register for read or write
  uint16_t    reg_data;      ///< data to write register or read from register
  union {
    uint8_t   config;
    struct {
    uint8_t   classifier :1; ///< config: classifier
    uint8_t   dummy_c    :7; ///< config: reserverd
    } config_bits;
  };
  uint8_t     length;        ///< amount of components in the vector
  uint8_t     answers;       ///< amount of answers we wait to get
  uint8_t     dummy0;        ///< reserve byte #0
  uint8_t     dummy1;        ///< reserve byte #1
  union {
    uint8_t   ncr;
    struct {
      uint8_t context   :7;  ///< ncr: context
      uint8_t dist_eval :1;  ///< ncr: distance eval
    } ncr_bits;
  };
  uint16_t    category;      ///< value of category
  uint16_t    maxif;         ///< value of aif
  uint16_t    minif;         ///< value of minif
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
    static_assert(std::is_pod<struct pcie_data_upack_t>::value, "pcie_data_upack_t is not POD");
#endif // __cplusplus
    static_assert((sizeof(struct pcie_data_upack_t) == 16),
                  "sizeof(struct pcie_data_upack_t) != 16");
// +-------------------------------------------------------------------------------------------+

/// extended pack = header (upack) + components
struct pcie_data_xpack_t
{
  struct pcie_data_upack_t upack;
  nn_vector_comp_t comp[NN_NEURON_COMPONENTS];
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
    static_assert(std::is_pod<struct pcie_data_xpack_t>::value, "pcie_data_xpack_t is not POD");
#endif // __cplusplus
    // hardcoded: for component type == uint8_t
    static_assert((sizeof(struct pcie_data_xpack_t) == 272),
                  "sizeof(struct pcie_data_xpack_t) != 272");
// +-------------------------------------------------------------------------------------------+

union nn_int_reg_io_t
{
  uint32_t   data;
  struct
  {
    uint8_t  opcode;  ///< code of operation - register read/write
    uint8_t  address; ///< register address
    uint16_t value;   ///< register value
  } part;
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
    static_assert(std::is_pod<union nn_int_reg_io_t>::value, "nn_int_reg_io_t is not POD");
#endif // __cplusplus
    static_assert((sizeof(union nn_int_reg_io_t) == 4),
                  "sizeof(union nn_int_reg_io_t) != 4");
// +-------------------------------------------------------------------------------------------+

union rx_data_learn_t
{
  uint64_t      data;
  struct
  {
    uint8_t     opcode;       ///< code of operation
    uint8_t     dummy0;
    uint16_t    category;     ///< category of learn vector
    uint16_t    ncount;       ///< amount of commited neurons
    uint16_t    dummy1;
  } part;
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
  static_assert(std::is_pod<union rx_data_learn_t>::value,
                "union rx_data_learn_t is not POD");
#endif // __cplusplus
  static_assert((sizeof(union rx_data_learn_t) == 8),
                "sizeof(union rx_data_learn_t) != 8");
// +-------------------------------------------------------------------------------------------+

struct rx_data_class_t
{
  uint8_t                        opcode;
  uint8_t                        ncount:    6;
  uint8_t                        UNC:       1;
  uint8_t                        ID:        1;
  struct response_neuron_state_t data[NN_MAX_RESP_COUNT];
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
    static_assert(std::is_pod<struct rx_data_class_t>::value, "rx_data_class_t is not POD");
#endif // __cplusplus
    static_assert((sizeof(struct rx_data_class_t) == 512),
                  "sizeof(struct rx_data_class_t) != 512");
// +-------------------------------------------------------------------------------------------+

struct rx_data_load_t
{
  uint8_t                        opcode;
  uint8_t                        dummy0;
  uint16_t                       neurons_restored;
};
// +--------------------------------+ static checks +------------------------------------------+
#ifdef __cplusplus
    static_assert(std::is_pod<struct rx_data_load_t>::value, "rx_data_load_t is not POD");
#endif // __cplusplus
    static_assert((sizeof(struct rx_data_load_t) == 4),
                  "sizeof(struct rx_data_load_t) != 4");
// +-------------------------------------------------------------------------------------------+

#pragma pack(pop)

// *INDENT-ON*
// clang-format on

#endif  // ONCE_INC_NTIA_API_DATA_TYPES_LL_H_
