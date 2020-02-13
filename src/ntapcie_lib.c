/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef NTIAPCIE_DEBUG
#include <stdio.h>
#endif // NTIAPCIE_DEBUG

#include "ntia_api_data_types.h"
#include "ntia_api.h"
#include "ntia_api_data_types_ll.h"
#include "ntia_api_ll.h"

#include "ntapcie_int.h"

#include "pcie/transport_pcie.h"

static struct pcie_io_handle_t _loc_io_handle;

// public library functions ------------------------------------------------------------
enum ntpcie_nn_error_t NTIA_API ntpcie_sys_init(struct nta_dev_handle_t  * const dev_handle,
                                                struct nta_pcidev_list_t * const devs_list)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  if (dev_handle == NULL)
  {
    return NTPCIE_ERROR_ARGS_NULL_POINTER;
  }

  devs_list_clear(devs_list);

  _loc_io_handle._iox_handle = NULL;
  _loc_io_handle._u32x_space = NTIA_PCIE_INVALID_SP;

  io_result = ntia_pcie_io_init(&_loc_io_handle);

  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    dev_handle->_iox_handle = &_loc_io_handle;
    dev_handle_hash_update(dev_handle);
    io_result = ntia_pcie_io_device_scan(dev_handle->_iox_handle, devs_list);
    nn_result = NTPCIE_ERROR_SUCCESS;
  }
  else
  {
    _loc_io_handle._iox_handle = NULL;
    _loc_io_handle._u32x_space = NTIA_PCIE_INVALID_SP;
    dev_handle_invalidate(dev_handle);
    nn_result = NTPCIE_ERROR_UNKNOWN;
  }

  nn_state_reset(&dev_handle->nn_state);

  return nn_result;
}

// Close handle to a PCI device
enum ntpcie_nn_error_t NTIA_API ntpcie_sys_deinit(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    return NTPCIE_ERROR_INVALID_HANDLE;
  }

  io_result = ntia_pcie_io_deinit(&_loc_io_handle);
  nn_state_reset(&dev_handle->nn_state);
  dev_handle_invalidate(dev_handle);

  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    nn_result = NTPCIE_ERROR_SUCCESS;
  }
  else
  {
    nn_result = NTPCIE_ERROR_UNKNOWN;
  }

  return nn_result;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_device_open(struct nta_dev_handle_t * const dev_handle,
                                                   const uint16_t pci_bus,
                                                   const uint16_t pci_slot,
                                                   const uint16_t pci_func)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  enum ntpcie_nn_error_t nn_result;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    return NTPCIE_ERROR_INVALID_HANDLE;
  }

  io_result = ntia_pcie_io_device_open(dev_handle->_iox_handle, pci_bus, pci_slot, pci_func);
  if (io_result != NTPCIE_IO_ERROR_SUCCESS)
  {
    return NTPCIE_ERROR_CARD_OPEN;
  }

  nn_result = ntpcie_device_reset(dev_handle); // ... and read amount of neurons
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    return NTPCIE_ERROR_CARD_RESET;
  }

  return NTPCIE_ERROR_SUCCESS;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_device_close(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    return NTPCIE_ERROR_INVALID_HANDLE;
  }

  io_result = ntia_pcie_io_device_close(dev_handle->_iox_handle);
  if (io_result != NTPCIE_IO_ERROR_SUCCESS)
  {
    return NTPCIE_ERROR_UNKNOWN;
  }

  nn_state_reset(&dev_handle->nn_state);

  return NTPCIE_ERROR_SUCCESS;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_device_reset(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }

  nn_state_reset(&dev_handle->nn_state);

  nn_result = ntpcie_card_reset(dev_handle); // NB: ... and get neurons_overall from card and set in dev_handle info struct

ret_result:
  return nn_result;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_nn_reset(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_nn_error_t nn_result;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }

  dev_handle->nn_state.kbase_id          = 0;
  dev_handle->nn_state.neurons_committed = 0;

  nn_result = ntpcie_nn_register_write(dev_handle, CM_FORGET, 0x0000u);

ret_result:
  return nn_result;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_nn_register_read(struct nta_dev_handle_t* const dev_handle,
                                                        const enum nn_int_register_t reg_address,
                                                        uint16_t* const reg_value)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  struct pcie_data_upack_t tx_data;
  union nn_int_reg_io_t rx_data;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }
  else if (reg_value == NULL)
  {
    nn_result = NTPCIE_ERROR_ARGS_NULL_POINTER;
    goto ret_result;
  }

  memset(&tx_data, 0, sizeof(tx_data));

  tx_data.opcode      = NTPCIE_OC_REG_READ;
  tx_data.reg_address = (uint8_t)reg_address;

  nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    goto ret_result;
  }

  io_result = ntia_pcie_io_device_mem_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &tx_data, sizeof(tx_data));
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    union pcie_card_status_t dev_status;

    nn_result = ntpcie_card_wait_ready_data(dev_handle, NTPCIE_MAX_CYCLES_STD, &dev_status);
    if (nn_result != NTPCIE_ERROR_SUCCESS)
    {
      goto ret_result;
    }

    io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &rx_data.data);
    if (io_result != NTPCIE_IO_ERROR_SUCCESS)
    {
      nn_result = NTPCIE_ERROR_SERV_READ;
      goto ret_result;
    }
    else if (rx_data.part.opcode == tx_data.opcode && rx_data.part.address == tx_data.reg_address)
    {
      *reg_value = rx_data.part.value;
      nn_result  = NTPCIE_ERROR_SUCCESS;
      goto ret_result;
    }
    else
    {
      nn_result = NTPCIE_ERROR_REG_READ;
      goto ret_result;
    }
  }
  else
  {
    nn_result = NTPCIE_ERROR_REG_READ;
    goto ret_result;
  }

ret_result:
  return nn_result;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_nn_register_write(struct nta_dev_handle_t* const dev_handle,
                                                         const enum nn_int_register_t reg_address,
                                                         const uint16_t reg_value)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }

  struct pcie_data_upack_t tx_data;
  union nn_int_reg_io_t rx_data;

  memset(&tx_data, 0, sizeof(tx_data));

  tx_data.opcode      = NTPCIE_OC_REG_WRITE;
  tx_data.reg_address = (uint8_t)reg_address;
  tx_data.reg_data    = reg_value;

  nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    goto ret_result;
  }

  // write data to memory
  io_result = ntia_pcie_io_device_mem_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &tx_data, sizeof(tx_data));
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    union pcie_card_status_t dev_status;
    for (size_t cnt = 0; cnt < NTPCIE_MAX_CYCLES_STD; ++cnt)
    {
      // read status register
      io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
      if (io_result != NTPCIE_IO_ERROR_SUCCESS)
      {
        nn_result = NTPCIE_ERROR_SERV_READ;
        goto ret_result;
      }
      // check data ready
      if (dev_status.part.results_ready == 1)
      {
        // read data from memory
        io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &rx_data.data);
        if (io_result != NTPCIE_IO_ERROR_SUCCESS)
        {
          nn_result = NTPCIE_ERROR_SERV_READ;
          goto ret_result;
        }
        else if (rx_data.part.opcode == tx_data.opcode && rx_data.part.address == tx_data.reg_address)
        {
          nn_result = NTPCIE_ERROR_SUCCESS;
          goto ret_result;
        }
      }
    }
    nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
    goto ret_result;
  }
  else
  {
    nn_result = NTPCIE_ERROR_REG_WRITE;
    goto ret_result;
  }

ret_result:
  return nn_result;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_nn_vector_learn(struct nta_dev_handle_t* const dev_handle,
                                                       const enum nn_dist_eval_t dist_eval,
                                                       const uint16_t context,
                                                       const uint16_t category,
                                                       const uint16_t maxif,
                                                       const uint16_t minif,
                                                       const size_t comps_count,
                                                       const nn_vector_comp_t data_vector[])
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  uint16_t bytes                   = 0;
  uint32_t pack_size_bytes         = 0;

  union pcie_card_status_t dev_status;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }
  else if (data_vector == NULL)
  {
    nn_result = NTPCIE_ERROR_ARGS_NULL_POINTER;
    goto ret_result;
  }
  else if ((comps_count < 1) || (comps_count > NN_NEURON_COMPONENTS))
  {
    nn_result = NTPCIE_ERROR_ARGS_COMPS_COUNT;
    goto ret_result;
  }
  else if ((dist_eval != NN_DIST_EVAL_L1) && (dist_eval != NN_DIST_EVAL_LSUP))
  {
    nn_result = NTPCIE_ERROR_ARGS_DIST_EVAL;
    goto ret_result;
  }
  else if ((context < 1) || (context > 127))
  {
    nn_result = NTPCIE_ERROR_ARGS_CONTEXT;
    goto ret_result;
  }
  else if (category > 32766)
  {
    nn_result = NTPCIE_ERROR_ARGS_CATEGORY;
    goto ret_result;
  }

  struct pcie_data_xpack_t tx_data;
  union rx_data_learn_t rx_data;

  uint32_t comps_size_bytes = (uint32_t)(sizeof(tx_data.comp[0]) * comps_count);

  // build data pack (header) to send PCIe card
  tx_data.upack.opcode                 = NTPCIE_OC_VECTOR_LEARN;
  tx_data.upack.config_bits.classifier = NN_CLASSIFIER_RBF;
  tx_data.upack.config_bits.dummy_c    = 0;
  tx_data.upack.ncr_bits.context       = (context & 0x7Fu);
  tx_data.upack.ncr_bits.dist_eval     = dist_eval;
  tx_data.upack.category               = category;
  tx_data.upack.maxif                  = maxif;
  tx_data.upack.minif                  = minif;
  tx_data.upack.length                 = (uint8_t)(comps_count - 1);

  // copy components
  memcpy(&tx_data.comp[0], &data_vector[0], comps_size_bytes);

  // calculate size in bytes to send
  pack_size_bytes = sizeof(tx_data.upack) + comps_size_bytes;

  // HACK: workaround: componets count (sizeof *data_pack) MUST be multiple 4 (bytes)
  uint16_t pack_size_m4_remains = pack_size_bytes & 0x0003u;
  if (pack_size_m4_remains > 0)
  {
    pack_size_bytes += (4 - pack_size_m4_remains);
  }
  // -----------------------------------------------------------------

  if (pack_size_bytes > sizeof(tx_data))
  {
    nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
    goto ret_result;
  }

  nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    goto ret_result;
  }

  uint64_t cpu_cycles_start = _cpu_get_tick_count();

  io_result = ntia_pcie_io_device_mem_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &tx_data, pack_size_bytes);
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    for (size_t cnt = 0; cnt < NTPCIE_MAX_CYCLES_STD; ++cnt)
    {
      // read status register
      io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
      if (io_result != NTPCIE_IO_ERROR_SUCCESS)
      {
        nn_result = NTPCIE_ERROR_SERV_READ;
        goto ret_result;
      }
      // check data ready and net ready
      if (dev_status.part.results_ready == 1)
      {
        bytes = dev_status.part.result_size * NTPCIE_DATA_BLOCK_SIZE;
        // we have needly amount bytes to read
        if (bytes == sizeof(rx_data))
        {
          // read data from PCIe card
          io_result = ntia_pcie_io_device_mem_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &rx_data.data, bytes);
          if (io_result != NTPCIE_IO_ERROR_SUCCESS)
          {
            nn_result = NTPCIE_ERROR_DATA_READ;
            goto ret_result;
          }

          uint64_t cpu_cycles_stop = _cpu_get_tick_count();

          if (rx_data.part.ncount == 0xFFFFu)
          {
            if (dev_handle->nn_state.neurons_committed != dev_handle->nn_state.neurons_overall)
            {
              dev_handle->nn_state.neurons_committed = dev_handle->nn_state.neurons_overall;
            }
          }
          else
          {
            dev_handle->nn_state.neurons_committed = rx_data.part.ncount;
          }

          // update performance counters
          ++(dev_handle->nn_state.vecs_count_total_learn);
          dev_handle->nn_state.cpu_ticks_last_oper = cpu_cycles_stop - cpu_cycles_start;
          dev_handle->nn_state.cpu_ticks_total_learn += dev_handle->nn_state.cpu_ticks_last_oper;
          dev_handle->nn_state.count_loop_wait_ready = cnt;

          nn_result = NTPCIE_ERROR_SUCCESS;
          goto ret_result;
        }
        // we havn't needly amount bytes to read
        else if (bytes > 0)
        {
          nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
          goto ret_result;
        }
        // we havn't bytes to read
        else
        {
          nn_result = NTPCIE_ERROR_NO_DATA_FOR_READ;
          goto ret_result;
        }
      }
    }
    // wait time is out
    nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
    goto ret_result;
  }
  else
  {
    nn_result = NTPCIE_ERROR_DATA_WRITE;
    goto ret_result;
  }

ret_result:
  return nn_result;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_nn_vector_classify(struct nta_dev_handle_t* const dev_handle,
                                                          const enum nn_dist_eval_t dist_eval,
                                                          const uint16_t context,
                                                          const enum nn_classifier_t classifier,
                                                          const size_t comps_count,
                                                          const nn_vector_comp_t data_vector[],
                                                          size_t* const number_of_responses,
                                                          struct response_neuron_state_t resp[])
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  uint16_t bytes                   = 0;
  uint32_t pack_size_bytes         = 0;

  union pcie_card_status_t dev_status;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }
  else if ((data_vector == NULL) || (number_of_responses == NULL) || (resp == NULL))
  {
    nn_result = NTPCIE_ERROR_ARGS_NULL_POINTER;
    goto ret_result;
  }
  else if ((comps_count < 1) || (comps_count > NN_NEURON_COMPONENTS))
  {
    nn_result = NTPCIE_ERROR_ARGS_COMPS_COUNT;
    goto ret_result;
  }
  else if ((dist_eval != NN_DIST_EVAL_L1) && (dist_eval != NN_DIST_EVAL_LSUP))
  {
    nn_result = NTPCIE_ERROR_ARGS_DIST_EVAL;
    goto ret_result;
  }
  else if ((context < 1) || (context > 127))
  {
    nn_result = NTPCIE_ERROR_ARGS_CONTEXT;
    goto ret_result;
  }
  else if ((classifier != NN_CLASSIFIER_KNN) && (classifier != NN_CLASSIFIER_RBF))
  {
    nn_result = NTPCIE_ERROR_ARGS_CLASSIFIER;
    goto ret_result;
  }
  else if ((*number_of_responses < 1) || (*number_of_responses > NN_MAX_RESP_COUNT))
  {
    nn_result = NTPCIE_ERROR_ARGS_RESP_COUNT;
    goto ret_result;
  }

  struct pcie_data_xpack_t tx_data;
  struct rx_data_class_t rx_data;

  uint32_t comps_size_bytes = (uint32_t)(sizeof(tx_data.comp[0]) * comps_count);

  // build data pack (header) to send PCIe card
  tx_data.upack.opcode                 = NTPCIE_OC_VECTOR_CLASSIFY;
  tx_data.upack.config_bits.classifier = (classifier & 0x01u);
  tx_data.upack.config_bits.dummy_c    = 0;
  tx_data.upack.ncr_bits.context       = (context & 0x7Fu);
  tx_data.upack.ncr_bits.dist_eval     = dist_eval;
  // MAX resp's
  tx_data.upack.answers = (uint8_t)(*number_of_responses);
  // set components count
  tx_data.upack.length = (uint8_t)(comps_count - 1);
  // unused in this mode
  tx_data.upack.category = 0;
  tx_data.upack.maxif    = 0;
  tx_data.upack.minif    = 0;

  // copy components
  memcpy(&tx_data.comp[0], &data_vector[0], comps_size_bytes);

  // calculate size in bytes to send
  pack_size_bytes = sizeof(tx_data.upack) + comps_size_bytes;

  // HACK: workaround: componets count (sizeof *data_pack) MUST be multiple 4 (bytes)
  uint16_t pack_size_m4_remains = pack_size_bytes & 0x0003u;
  if (pack_size_m4_remains > 0)
  {
    pack_size_bytes += (4 - pack_size_m4_remains);
  }
  // -----------------------------------------------------------------

  if (pack_size_bytes > sizeof(tx_data))
  {
    nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
    goto ret_result;
  }

  nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    goto ret_result;
  }

  uint64_t cpu_cycles_start = _cpu_get_tick_count();

#ifdef NTIAPCIE_DEBUG
  puts(" *** NTIAPCIE_DEBUG active");
  fprintf(stderr, "----- (1) nresp = %zu\n", *number_of_responses);
#endif // NTIAPCIE_DEBUG

  // write data to memory
  io_result = ntia_pcie_io_device_mem_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &tx_data, pack_size_bytes);
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    for (size_t cnt = 0; cnt < NTPCIE_MAX_CYCLES_STD; ++cnt)
    {
      // read status register
      io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
      if (io_result != NTPCIE_IO_ERROR_SUCCESS)
      {
        nn_result = NTPCIE_ERROR_SERV_READ;
        goto ret_result;
      }
      // check data ready and net ready
      if (dev_status.part.results_ready == 1)
      {
        bytes = dev_status.part.result_size * NTPCIE_DATA_BLOCK_SIZE;
        // we have needly amount bytes to read
        if (bytes == 0 || bytes > sizeof(rx_data))
        {
          nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
          goto ret_result;
        }
        else
        {
          // read data from memory
          io_result = ntia_pcie_io_device_mem_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &rx_data, bytes);
          if (io_result != NTPCIE_IO_ERROR_SUCCESS)
          {
            nn_result = NTPCIE_ERROR_DATA_READ;
            goto ret_result;
          }

          if (rx_data.opcode == NTPCIE_OC_FAULT)
          {
            nn_result = NTPCIE_ERROR_CARD_FAULT;
            goto ret_result;
          }

          uint64_t cpu_cycles_stop = _cpu_get_tick_count();
          // update performance counters
          ++(dev_handle->nn_state.vecs_count_total_class);
          dev_handle->nn_state.cpu_ticks_last_oper = cpu_cycles_stop - cpu_cycles_start;
          dev_handle->nn_state.cpu_ticks_total_class += dev_handle->nn_state.cpu_ticks_last_oper;
          dev_handle->nn_state.count_loop_wait_ready = cnt;

          // make results to return
          size_t real_number_of_responses = (*number_of_responses > rx_data.ncount) ? rx_data.ncount : *number_of_responses;
          for (size_t ix = 0; ix < real_number_of_responses; ++ix)
          {
            uint16_t distance;
            distance = rx_data.data[ix].distance;
            if (distance == 0xFFFFu)
            {
              real_number_of_responses = ix;
              break;
            }
            resp[ix].id          = rx_data.data[ix].id;
            resp[ix].category    = rx_data.data[ix].category;
            resp[ix].degenerated = rx_data.data[ix].degenerated;
            resp[ix].distance    = distance;
          }
          // set final responses count
          *number_of_responses = real_number_of_responses;

#ifdef NTIAPCIE_DEBUG
          puts(" *** NTIAPCIE_DEBUG active");
          fprintf(stderr, "----- (2) nresp = %zu\n", *number_of_responses);
#endif // NTIAPCIE_DEBUG

          nn_result = NTPCIE_ERROR_SUCCESS;
          goto ret_result;
        }
      }
    }
    // wait time is out
    nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
    goto ret_result;
  }
  else
  {
    nn_result = NTPCIE_ERROR_DATA_WRITE;
    goto ret_result;
  }

ret_result:
  return nn_result;
}

enum ntpcie_nn_error_t NTIA_API ntpcie_nn_neuron_read(struct nta_dev_handle_t* const dev_handle,
                                                      const uint16_t ix_neuron,
                                                      struct nn_neuron_t* const _neuron)
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  uint16_t bytes                   = 0;

  union pcie_card_status_t dev_status;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }
  else if (_neuron == NULL)
  {
    nn_result = NTPCIE_ERROR_ARGS_NULL_POINTER;
    goto ret_result;
  }
#ifdef NTIAPCIE_DEBUG
  puts(" *** NTIAPCIE_DEBUG active");
#else
  else if (ix_neuron > dev_handle->nn_state.neurons_overall)
  {
    nn_result = NTPCIE_ERROR_ARGS_COMPS_COUNT;
    goto ret_result;
  }
#endif // NTIAPCIE_DEBUG

  struct pcie_data_upack_t tx_data;

  memset(&tx_data, 0, sizeof(tx_data));

  tx_data.opcode   = NTPCIE_OC_NEURON_READ;
  tx_data.reg_data = ix_neuron;

  nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    goto ret_result;
  }

  // write data to memory
  io_result = ntia_pcie_io_device_mem_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &tx_data, sizeof(tx_data));
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    for (size_t cnt = 0; cnt < NTPCIE_MAX_CYCLES_STD; ++cnt)
    {
      // read status register
      io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
      if (io_result != NTPCIE_IO_ERROR_SUCCESS)
      {
        nn_result = NTPCIE_ERROR_SERV_READ;
        goto ret_result;
      }
      // check data ready and net ready
      if (dev_status.part.results_ready == 1)
      {
        bytes = dev_status.part.result_size * NTPCIE_DATA_BLOCK_SIZE;
        // we have bytes to read
        if (bytes > 0)
        {
          if (bytes > sizeof(*_neuron))
          {
            nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
            goto ret_result;
          }
          // read data from memory
          io_result = ntia_pcie_io_device_mem_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, _neuron, sizeof(*_neuron));
          if (io_result == NTPCIE_IO_ERROR_SUCCESS)
          {
            nn_result = NTPCIE_ERROR_SUCCESS;
            goto ret_result;
          }
          else
          {
            nn_result = NTPCIE_ERROR_DATA_READ;
            goto ret_result;
          }
        }
        // we have not bytes to read
        else
        {
          nn_result = NTPCIE_ERROR_NO_DATA_FOR_READ;
          goto ret_result;
        }
      }
    }
    // wait time is out
    nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
    goto ret_result;
  }
  else
  {
    nn_result = NTPCIE_ERROR_DATA_WRITE;
    goto ret_result;
  }

ret_result:
  return nn_result;
}

// transfer KB from NN to struct _neuron in main memory (RAM)
enum ntpcie_nn_error_t NTIA_API ntpcie_kbase_store(struct nta_dev_handle_t* const dev_handle, struct nn_neuron_t* const _neuron)
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  uint16_t bytes                   = 0;

  union pcie_card_status_t dev_status;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }
  else if (_neuron == NULL)
  {
    nn_result = NTPCIE_ERROR_ARGS_NULL_POINTER;
    goto ret_result;
  }

  struct pcie_data_upack_t tx_data;

  memset(&tx_data, 0, sizeof(tx_data));

  tx_data.opcode = NTPCIE_OC_KBASE_STORE;

  nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    goto ret_result;
  }

  uint64_t cpu_cycles_start = _cpu_get_tick_count();

  // write data to PCIe card
  io_result = ntia_pcie_io_device_mem_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &tx_data, sizeof(tx_data));
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    for (size_t cnt = 0; cnt < NTPCIE_MAX_CYCLES_STD; ++cnt)
    {
      // read status register
      io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
      if (io_result != NTPCIE_IO_ERROR_SUCCESS)
      {
        nn_result = NTPCIE_ERROR_SERV_READ;
        goto ret_result;
      }
      // check data ready and net ready
      if (dev_status.part.results_ready == 1)
      {
        bytes = dev_status.part.result_size * NTPCIE_DATA_BLOCK_SIZE;
        // we have bytes to read
        if (bytes > 0)
        {
          if (bytes != sizeof(*_neuron))
          {
            nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
            goto ret_result;
          }
          // invalidate neuron storage
          memset(_neuron->comp, 0xFFu, sizeof(_neuron->comp));
          // read data from memory
          io_result = ntia_pcie_io_device_mem_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, _neuron, sizeof(*_neuron));
          if (io_result == NTPCIE_IO_ERROR_SUCCESS)
          {
            uint64_t cpu_cycles_stop = _cpu_get_tick_count();

            // update performance counters
            dev_handle->nn_state.cpu_ticks_last_oper   = cpu_cycles_stop - cpu_cycles_start;
            dev_handle->nn_state.count_loop_wait_ready = cnt;

#ifdef NTIAPCIE_DEBUG
  puts(" *** NTIAPCIE_DEBUG active");
  fprintf(stderr, " DEBUG:str: ncr %u; cat %u; aif %u; minif %u\n",
          _neuron->ncr, _neuron->category,
          _neuron->aif, _neuron->minif);
#endif // NTIAPCIE_DEBUG

            nn_result = NTPCIE_ERROR_SUCCESS;
            goto ret_result;
          }
          else
          {
            nn_result = NTPCIE_ERROR_DATA_READ;
            goto ret_result;
          }
        }
        // we have not bytes to read
        else
        {
          nn_result = NTPCIE_ERROR_NO_DATA_FOR_READ;
          goto ret_result;
        }
      }
    }
    // wait time is out
    nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
    goto ret_result;
  }
  else
  {
    nn_result = NTPCIE_ERROR_DATA_WRITE;
    goto ret_result;
  }

ret_result:
  return nn_result;
}

// transfer KB from struct _neuron in main memory (RAM) to NN
enum ntpcie_nn_error_t NTIA_API ntpcie_kbase_load(struct nta_dev_handle_t* const dev_handle,
                                                  const size_t comps_count,
                                                  const struct nn_neuron_t* const _neuron)
{
  enum ntpcie_nn_error_t nn_result = NTPCIE_ERROR_SUCCESS;
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  uint16_t bytes                   = 0;
  uint32_t pack_size_bytes         = 0;

  union pcie_card_status_t dev_status;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }
  else if (_neuron == NULL)
  {
    nn_result = NTPCIE_ERROR_ARGS_NULL_POINTER;
    goto ret_result;
  }
  else if ((comps_count < 1) || (comps_count > NN_NEURON_COMPONENTS))
  {
    nn_result = NTPCIE_ERROR_ARGS_COMPS_COUNT;
    goto ret_result;
  }

  struct pcie_data_xpack_t tx_data;
  struct rx_data_load_t    rx_data;

  memset(&tx_data, 0, sizeof(tx_data));

  const uint32_t comps_size_bytes = (uint32_t)(sizeof(tx_data.comp[0]) * comps_count);

  // build data pack (header) to send PCIe card
  tx_data.upack.opcode    = NTPCIE_OC_KBASE_LOAD;
  tx_data.upack.ncr       = _neuron->ncr;
  tx_data.upack.category  = _neuron->category;
  tx_data.upack.maxif     = _neuron->aif;
  tx_data.upack.minif     = _neuron->minif;
  tx_data.upack.length    = (uint8_t)(comps_count - 1);

  // copy components
  memcpy(&tx_data.comp[0], &_neuron->comp[0], comps_size_bytes);

#ifdef NTIAPCIE_DEBUG
  puts(" *** NTIAPCIE_DEBUG active");
  fprintf(stderr, " DEBUG:ldr: ncr %u; cat %u; aif %u; minif %u\n",
          tx_data.upack.ncr, tx_data.upack.category,
          tx_data.upack.maxif, tx_data.upack.minif);
#endif // NTIAPCIE_DEBUG

  // calculate size in bytes to send
  pack_size_bytes = sizeof(tx_data.upack) + comps_size_bytes;

  // HACK: workaround: componets count (sizeof *data_pack) MUST be multiple 4 (bytes)
  uint16_t pack_size_m4_remains = pack_size_bytes & 0x0003u;
  if (pack_size_m4_remains > 0)
  {
    pack_size_bytes += (4 - pack_size_m4_remains);
  }
  // -----------------------------------------------------------------

  if (pack_size_bytes > sizeof(tx_data))
  {
    nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
    goto ret_result;
  }

  nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
  if (nn_result != NTPCIE_ERROR_SUCCESS)
  {
    goto ret_result;
  }

  uint64_t cpu_cycles_start = _cpu_get_tick_count();

  io_result = ntia_pcie_io_device_mem_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &tx_data, pack_size_bytes);
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    for (size_t cnt = 0; cnt < NTPCIE_MAX_CYCLES_STD; ++cnt)
    {
      // read status register
      io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
      if (io_result != NTPCIE_IO_ERROR_SUCCESS)
      {
        nn_result = NTPCIE_ERROR_SERV_READ;
        goto ret_result;
      }
      // check data ready and net ready
      if (dev_status.part.results_ready == 1)
      {
        bytes = dev_status.part.result_size * NTPCIE_DATA_BLOCK_SIZE;
        // we have needly amount bytes to read
        if (bytes == sizeof(rx_data))
        {
          // read data from PCIe card
          io_result = ntia_pcie_io_device_mem_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_DATA, &rx_data, bytes);
          if (io_result != NTPCIE_IO_ERROR_SUCCESS)
          {
            nn_result = NTPCIE_ERROR_DATA_READ;
            goto ret_result;
          }

          uint64_t cpu_cycles_stop = _cpu_get_tick_count();

          // ATT: neurons_restored==neurons_commited (*current* value of NN internal register NCOUNT)
          if (rx_data.neurons_restored == 0xFFFFu)
          {
            if (dev_handle->nn_state.neurons_committed != dev_handle->nn_state.neurons_overall)
            {
              dev_handle->nn_state.neurons_committed = dev_handle->nn_state.neurons_overall;
            }
          }
          else
          {
            dev_handle->nn_state.neurons_committed = rx_data.neurons_restored;
          }

          // update performance counters
          dev_handle->nn_state.cpu_ticks_last_oper   = cpu_cycles_stop - cpu_cycles_start;
          dev_handle->nn_state.count_loop_wait_ready = cnt;

          nn_result = NTPCIE_ERROR_SUCCESS;
          goto ret_result;
        }
        // we havn't needly amount bytes to read
        else if (bytes > 0)
        {
          nn_result = NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH;
          goto ret_result;
        }
        // we havn't bytes to read
        else
        {
          nn_result = NTPCIE_ERROR_NO_DATA_FOR_READ;
          goto ret_result;
        }
      }
    }
    // wait time is out
    nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
    goto ret_result;
  }
  else
  {
    nn_result = NTPCIE_ERROR_DATA_WRITE;
    goto ret_result;
  }

ret_result:
  return nn_result;
}

const char* ntpcie_error_text(enum ntpcie_nn_error_t const _ec)
{
  const char* _e_text;
  switch (_ec)
  {
    case NTPCIE_ERROR_SUCCESS:
      _e_text = "everything is OK";
      break;
    case NTPCIE_ERROR_UNKNOWN:
      _e_text = "error is unknown";
      break;
    case NTPCIE_ERROR_INVALID_HANDLE:
      _e_text = "device handle or address space number is wrong";
      break;
    case NTPCIE_ERROR_WAIT_TIMEOUT:
      _e_text = "waiting time is over";
      break;
    case NTPCIE_ERROR_IO:
      _e_text = "general IO error";
      break;
    case NTPCIE_ERROR_IO_MEMORY_SIZE_MISMATCH:
      _e_text = "IO size and memory area size mismatch";
      break;
    case NTPCIE_ERROR_CARD_RESET:
      _e_text = "error on card reset";
      break;
    case NTPCIE_ERROR_CARD_OPEN:
      _e_text = "error on car open";
      break;
    case NTPCIE_ERROR_CARD_FAULT:
      _e_text = "card fault";
      break;
    case NTPCIE_ERROR_DATA_READ:
      _e_text = "error reading from data area";
      break;
    case NTPCIE_ERROR_DATA_WRITE:
      _e_text = "error writing to data area";
      break;
    case NTPCIE_ERROR_REG_READ:
      _e_text = "error reading from NN internal register";
      break;
    case NTPCIE_ERROR_REG_WRITE:
      _e_text = "error writing to NN internal register";
      break;
    case NTPCIE_ERROR_SERV_READ:
      _e_text = "error reading from card service register";
      break;
    case NTPCIE_ERROR_SERV_WRITE:
      _e_text = "error writing to card service register";
      break;
    case NTPCIE_ERROR_NO_DATA_FOR_READ:
      _e_text = "no data bytes to read from card data area";
      break;
    case NTPCIE_ERROR_ARGS_NULL_POINTER:
      _e_text = "bad argument(s): pointer is NULL";
      break;
    case NTPCIE_ERROR_ARGS_COMPS_COUNT:
      _e_text = "bad argument(s): components count not in valid range";
      break;
    case NTPCIE_ERROR_ARGS_DIST_EVAL:
      _e_text = "bad argument(s): distance eval not in valid range";
      break;
    case NTPCIE_ERROR_ARGS_CLASSIFIER:
      _e_text = "bad argument(s): classifier type not in valid range";
      break;
    case NTPCIE_ERROR_ARGS_CONTEXT:
      _e_text = "bad argument(s): context not in valid range";
      break;
    case NTPCIE_ERROR_ARGS_CATEGORY:
      _e_text = "bad argument(s): category not in valid range";
      break;
    case NTPCIE_ERROR_ARGS_INFLUENCE_FIELDS:
      _e_text = "bad argument(s): influence fields not in valid range";
      break;
    case NTPCIE_ERROR_ARGS_RESP_COUNT:
      _e_text = "bad argument(s): response count not in valid range";
      break;
    case NTPCIE_ERROR_KBASE_EOF:
      _e_text = "knowledge base: end-of-file";
      break;
    case NTPCIE_ERROR_ITEMS_COUNT:
      _e_text = "placeholder";
      break;
  }
  return _e_text;
}
