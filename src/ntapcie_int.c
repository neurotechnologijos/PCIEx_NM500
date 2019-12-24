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

#include "ntia_api_data_types.h"
#include "ntia_api_data_types_ll.h"
#include "ntia_api_ll.h"

#include "ntapcie_int.h"

#include "pcie/transport_pcie.h"

void nn_state_reset(struct nn_state_t* const _state)
{
  _state->neurons_overall   = 0;
  _state->neurons_committed = 0;
  _state->kbase_id          = 0;
  // performance counters
  _state->count_loop_wait_ready  = 0;
  _state->cpu_ticks_last_oper    = 0;
  _state->cpu_ticks_total_learn  = 0;
  _state->cpu_ticks_total_class  = 0;
  _state->vecs_count_total_learn = 0;
  _state->vecs_count_total_class = 0;
}

enum ntpcie_nn_error_t ntpcie_card_wait_ready(const struct nta_dev_handle_t* const dev_handle,
                                              size_t wait_cycles,
                                              union pcie_card_status_t* const _status)
{
  enum ntpcie_io_error_t io_result;
  enum ntpcie_nn_error_t nn_result;
  union pcie_card_status_t dev_status;

  for (; wait_cycles > 0; --wait_cycles)
  {
    io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
    if (io_result != NTPCIE_IO_ERROR_SUCCESS)
    {
      nn_result = NTPCIE_ERROR_SERV_READ;
      goto ret_result;
    }
    else if (dev_status.part.fault == 1)
    {
      nn_result = NTPCIE_ERROR_CARD_FAULT;
      goto ret_result;
    }
    else if (dev_status.part.ready == 1)
    {
      nn_result = NTPCIE_ERROR_SUCCESS;
      goto ret_result;
    }
  }

  nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
  goto ret_result;

ret_result:
  if (_status != NULL)
  {
    *_status = dev_status;
  }

  return nn_result;
}

enum ntpcie_nn_error_t ntpcie_card_wait_ready_data(const struct nta_dev_handle_t* const dev_handle,
                                                   size_t wait_cycles,
                                                   union pcie_card_status_t* const _status)
{
  enum ntpcie_io_error_t io_result;
  enum ntpcie_nn_error_t nn_result;
  union pcie_card_status_t dev_status;

  for (; wait_cycles > 0; --wait_cycles)
  {
    io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_STATUS, &dev_status.data);
    if (io_result != NTPCIE_IO_ERROR_SUCCESS)
    {
      nn_result = NTPCIE_ERROR_SERV_READ;
      goto ret_result;
    }
    else if (dev_status.part.fault == 1)
    {
      nn_result = NTPCIE_ERROR_CARD_FAULT;
      goto ret_result;
    }
    else if (dev_status.part.results_ready == 1)
    {
      nn_result = NTPCIE_ERROR_SUCCESS;
      goto ret_result;
    }
  }

  nn_result = NTPCIE_ERROR_WAIT_TIMEOUT;
  goto ret_result;

ret_result:
  if (_status != NULL)
  {
    *_status = dev_status;
  }

  return nn_result;
}

enum ntpcie_nn_error_t ntpcie_card_reset(struct nta_dev_handle_t* const dev_handle)
{
  enum ntpcie_io_error_t io_result;
  enum ntpcie_nn_error_t nn_result;

  if (dev_handle_is_valid(dev_handle) != true)
  {
    nn_result = NTPCIE_ERROR_INVALID_HANDLE;
    goto ret_result;
  }

  io_result = ntia_pcie_io_device_wr32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_RESET, 0xDEADBEEFul);
  if (io_result == NTPCIE_IO_ERROR_SUCCESS)
  {
    // HACK: wait for TESTCAT is 2*ready
    nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
    nn_result = ntpcie_card_wait_ready(dev_handle, NTPCIE_MAX_CYCLES_STD, NULL);
    if (nn_result == NTPCIE_ERROR_SUCCESS)
    {
      uint32_t data = 0;
      // read amount of neurons in the neuron net
      io_result = ntia_pcie_io_device_rd32(dev_handle->_iox_handle, NTPCIE_DEVICE_ADDRESS_NET_INFO, &data);
      if (io_result == NTPCIE_IO_ERROR_SUCCESS)
      {
        dev_handle->nn_state.neurons_overall = (data & 0x0000FFFF);
        nn_result                            = NTPCIE_ERROR_SUCCESS;
        goto ret_result;
      }
    }
    goto ret_result;
  }
  else
  {
    nn_result = NTPCIE_ERROR_SERV_WRITE;
    goto ret_result;
  }

ret_result:
  return nn_result;
}
