/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_NTIA_API_H_
#define ONCE_INC_NTIA_API_H_

#include <stdint.h>
#include <stddef.h>

#include "ntia_api_data_types.h"

#ifndef CYTHON_ABI
  #include "ntia_shared_defs.h"
#else
  #define NTIA_API
#endif // CYTHON_ABI

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  /// global PCIe system control

  /**
   *  @brief      init NTIA NN system and PCIe card initialization
   *  @details    TODO ...
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @param[in]  devs_list pointer to nta_pcidev_list_t struct for get list of devices in system
   *              must be allocated in programm
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_sys_init(struct nta_dev_handle_t  * const dev_handle,
                                                  struct nta_pcidev_list_t * const devs_list);
  /**
   *  @brief      deinit NTIA NN system and PCIe card
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_sys_deinit(struct nta_dev_handle_t * const dev_handle);

  /// devices control

  /**
   *  @brief      PCIe card open
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @param[in]  pci_bus device address to select in PCI bus (from nta_pcidev_list_t)
   *  @param[in]  pci_slot device address to select in PCI bus (nta_pcidev_list_t)
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_device_open(struct nta_dev_handle_t * const dev_handle,
                                                     const uint16_t pci_bus,
                                                     const uint16_t pci_slot,
                                                     const uint16_t pci_func);
  /**
   *  @brief      PCIe card close
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_device_close(struct nta_dev_handle_t * const dev_handle);

  /**
   *  @brief      reset (hard) card and neuron net
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_device_reset(struct nta_dev_handle_t * const dev_handle);

  /// NN service functions
  /**
   *  @brief      reset (soft) neuron net (FORGET)
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_nn_reset(struct nta_dev_handle_t * const dev_handle);

  /// NN learn/classify functions

  /**
   *  @brief      Learn vector
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @param[in]  dist_eval
   *  @param[in]  context
   *  @param[in]  category
   *  @param[in]  maxif
   *  @param[in]  minif
   *  @param[in]  comps_count components count in vector
   *  @param[in]  data_vector[] array of components
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_nn_vector_learn(struct nta_dev_handle_t * const dev_handle,
                               const enum nn_dist_eval_t dist_eval,
                               const uint16_t context,
                               const uint16_t category,
                               const uint16_t maxif,
                               const uint16_t minif,
                               const size_t   comps_count,
                               const nn_vector_comp_t data_vector[]);

  /**
   *  @brief          Classify vector
   *  @details        TODO
   *  @param[in]      dev_handle pointer to structure with internal id's PCIe card
   *  @param[in]      dist_eval
   *  @param[in]      context
   *  @param[in]      classifier
   *  @param[in]      comps_count
   *  @param[in]      data_vector
   *  @param[in/out]  number_of_responses [in] - desired number of responses; [out] - real number of responses
   *  @param[out]     resp array with recognize results (must be at least number_of_responses[in] size)
   *  @return         status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_nn_vector_classify(struct nta_dev_handle_t * const dev_handle,
                               const enum nn_dist_eval_t dist_eval,
                               const uint16_t context,
                               const enum nn_classifier_t classifier,
                               const size_t comps_count,
                               const nn_vector_comp_t data_vector[],
                               size_t * const number_of_responses,
                               struct response_neuron_state_t resp[]);


  /// NN neuron read state

  /**
   *  @brief      read state of single neuron
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @param[in]  ix_neuron neuron's number that we want to read (valid range is [0..neurons_committed-1])
   *  @param[out] _neuron the pointer to structure neuron_t with data of neuron (all components will be filled)
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_nn_neuron_read(struct nta_dev_handle_t * const dev_handle,
                                                const uint16_t ix_neuron,
                                                struct nn_neuron_t* const _neuron);

  /// NN knowledge base (stored state) functions

// ------ ATTENTION: NN register NCOUNT must be read before storing KB (for implicit set NR mode in NN)
//   uint16_t _ncount = 0;
//   nn_result = ntpcie_nn_register_read(dev_handle, (enum nn_int_register_t)(CM_NCOUNT), &_ncount);
// -----------------------------------------------------------------

  /**
   *  @brief      transfer state of neurons from NN to main PC RAM as "knowledge base"
   *              function must be called sequentially many times (neurons_committed)
   *              without interruptions by calls to other "ntpcie_*" functions
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @param[out] _neuron the pointer to structure neuron_t with data of neuron (all (256) components will be filled)
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_kbase_store(struct nta_dev_handle_t * const dev_handle,
                                                     struct nn_neuron_t* const _neuron);

// ------ ATTENTION: NN register NCOUNT must be read before loading KB (for implicit set NR mode in NN)
//   uint16_t _ncount = 0;
//   nn_result = ntpcie_nn_register_read(dev_handle, (enum nn_int_register_t)(CM_NCOUNT), &_ncount);
// -----------------------------------------------------------------

  /**
   *  @brief      transfer state of neurons from main PC RAM to NN
   *              function must be called sequentially many times
   *              without interruptions by calls to other "ntpcie_*" functions
   *  @details    TODO
   *  @param[in]  dev_handle pointer to structure with internal id's PCIe card
   *  @param[in]  comps_count components count in current neuron to be restored
   *  @param[in]  _neuron the pointer to structure neuron_t with data of neuron
   *  @return     status of operation (NTPCIE_ERROR_...)
   */
  enum ntpcie_nn_error_t NTIA_API ntpcie_kbase_load(struct nta_dev_handle_t * const dev_handle,
                                                    const size_t comps_count,
                                                    const struct nn_neuron_t * const _neuron);


  const char *ntpcie_error_text(enum ntpcie_nn_error_t const _ec);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // ONCE_INC_NTIA_API_H_
