### language: cython
# cython: language_level=3
# -*- coding: utf-8 -*-

'''
/*
 * Copyright (c) 2017-2020 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */
'''

from libc.stdint cimport uint8_t, uint16_t, uint32_t, uint64_t
from libc.stddef cimport size_t

DEF NTIA_PCIE_MAX_CARDS  = 16

DEF NN_NEURON_COMPONENTS = 256

### values for classifiers
DEF CLASS_RBF         =              (0x0000)
DEF CLASS_KNN         =              (0x0020)

### values for metrics
DEF METRICS_L1        =              (0x0000)
DEF METRICS_LSUP      =              (0x0080)

### influence fields default values
DEF DEF_MAXIF         =              (0x4000)
DEF DEF_MINIF         =              (0x0002)


cdef extern from "../api/ntia_api_data_types.h" nogil:

  ctypedef uint8_t nn_vector_comp_t;

  cdef enum ntpcie_nn_error_t:
    NTPCIE_ERROR_SUCCESS               = 0x00000000
    NTPCIE_ERROR_UNKNOWN
    NTPCIE_ERROR_INVALID_HANDLE
    NTPCIE_ERROR_WAIT_TIMEOUT
    NTPCIE_ERROR_IO
    NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH
    NTPCIE_ERROR_CARD_RESET
    NTPCIE_ERROR_CARD_OPEN
    NTPCIE_ERROR_CARD_FAULT
    NTPCIE_ERROR_DATA_READ
    NTPCIE_ERROR_DATA_WRITE
    NTPCIE_ERROR_REG_READ
    NTPCIE_ERROR_REG_WRITE
    NTPCIE_ERROR_SERV_READ
    NTPCIE_ERROR_SERV_WRITE
    NTPCIE_ERROR_NO_DATA_FOR_READ
    NTPCIE_ERROR_ARGS_NULL_POINTER
    NTPCIE_ERROR_ARGS_COMPS_COUNT
    NTPCIE_ERROR_ARGS_DIST_EVAL
    NTPCIE_ERROR_ARGS_CLASSIFIER
    NTPCIE_ERROR_ARGS_CONTEXT
    NTPCIE_ERROR_ARGS_CATEGORY
    NTPCIE_ERROR_ARGS_INFLUENCE_FIELDS
    NTPCIE_ERROR_ARGS_RESP_COUNT
    NTPCIE_ERROR_KBASE_EOF

  cdef enum nn_classifier_t:
    NN_CLASSIFIER_RBF = 0x00
    NN_CLASSIFIER_KNN = 0x01

  cdef enum nn_dist_eval_t:
    NN_DIST_EVAL_L1   = 0x00
    NN_DIST_EVAL_LSUP = 0x01

  cdef struct nn_state_t:
    size_t               neurons_overall
    size_t               neurons_committed
    uint64_t             kbase_id
    size_t               count_loop_wait_ready
    size_t               cpu_ticks_last_oper
    size_t               cpu_ticks_total_learn
    size_t               cpu_ticks_total_class
    size_t               vecs_count_total_learn
    size_t               vecs_count_total_class
    size_t               neur_count_total_learn
    size_t               neur_count_total_class

  cdef struct nta_dev_handle_t:
    void*                _iox_handle
    uint32_t             _signature
    nn_state_t           nn_state

  cdef struct nta_pcidev_info_t:
    uint16_t             bus
    uint16_t             slot
    uint16_t             func

  cdef struct nta_pcidev_list_t:
    uint16_t             pci_id_vendor
    uint16_t             pci_id_device
    size_t               devs_count
    nta_pcidev_info_t    devices[NTIA_PCIE_MAX_CARDS]

  cdef packed struct response_neuron_state_t:
    uint16_t             distance
#   uint16_t             category     : 15
#   uint16_t             degenerated  : 1
    uint16_t             category
    uint16_t             id

  cdef packed struct nn_neuron_t:
    uint8_t              opcode
    uint8_t              ncr
    uint16_t             category
    uint16_t             aif
    uint16_t             minif
    nn_vector_comp_t     comp[NN_NEURON_COMPONENTS]



cdef extern from "../api/ntia_api.h" nogil:
  ntpcie_nn_error_t  ntpcie_sys_init(nta_dev_handle_t * dev_handle,
                               nta_pcidev_list_t * const devs_list)
  ntpcie_nn_error_t  ntpcie_sys_deinit(nta_dev_handle_t * dev_handle)
  ntpcie_nn_error_t  ntpcie_device_open(nta_dev_handle_t * dev_handle,
                               const uint16_t pci_bus,
                               const uint16_t pci_slot,
                               const uint16_t pci_func)

  ntpcie_nn_error_t  ntpcie_device_close(nta_dev_handle_t * dev_handle)
  ntpcie_nn_error_t  ntpcie_device_reset(nta_dev_handle_t * dev_handle)

  ntpcie_nn_error_t  ntpcie_nn_reset(nta_dev_handle_t * dev_handle)
  ntpcie_nn_error_t  ntpcie_nn_vector_learn(nta_dev_handle_t * dev_handle,
                               nn_dist_eval_t dist_eval,
                               uint16_t context,
                               uint16_t category,
                               uint16_t maxif,
                               uint16_t minif,
                               size_t   comps_count,
                               nn_vector_comp_t data_vector[])

  ntpcie_nn_error_t  ntpcie_nn_vector_classify(nta_dev_handle_t * dev_handle,
                               nn_dist_eval_t dist_eval,
                               uint16_t context,
                               nn_classifier_t classifier,
                               size_t comps_count,
                               nn_vector_comp_t data_vector[],
                               size_t * number_of_responses,
                               response_neuron_state_t resp[])


  ntpcie_nn_error_t  ntpcie_nn_neuron_read(nta_dev_handle_t * const dev_handle,
                               const uint16_t ix_neuron,
                               nn_neuron_t*  const _neuron)

  ntpcie_nn_error_t  ntpcie_kbase_store(nta_dev_handle_t * const dev_handle, nn_neuron_t* const _neuron)
  ntpcie_nn_error_t  ntpcie_kbase_load(nta_dev_handle_t * const dev_handle, const size_t comps_count, const nn_neuron_t * const _neuron)
