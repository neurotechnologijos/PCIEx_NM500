/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_NTIA_PCIE_API_LL_H_
#define ONCE_INC_NTIA_PCIE_API_LL_H_

#include <stdint.h>

#include "ntia_shared_defs.h"
#include "ntia_api_data_types.h"
#include "ntia_api_data_types_ll.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/// NN low level register's control

/**
 *  @brief      Read data from NN register
 *  @details    TODO
 *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
 *  @param[in]  reg_address register's address
 *  @param[out] reg_value poiter to uint16_t with result
 *  @return     status of operation (NTPCIE_ERROR_...)
 */
enum ntpcie_nn_error_t NTIA_API ntpcie_nn_register_read(struct nta_dev_handle_t* const dev_handle,
                                                const enum nn_int_register_t reg_address,
                                                uint16_t * const reg_value);

/**
 *  @brief      Write data to NN register
 *  @details    TODO
 *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
 *  @param[in]  reg_address register's address
 *  @param[in]  reg_value value to be write
 *  @param[in]  result poiter to structure with result
 *  @return     status of operation (NTPCIE_ERROR_...)
 */
enum ntpcie_nn_error_t NTIA_API ntpcie_nn_register_write(struct nta_dev_handle_t* const dev_handle,
                                                 const enum nn_int_register_t reg_address,
                                                 const uint16_t reg_value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // ONCE_INC_NTIA_PCIE_API_LL_H_
