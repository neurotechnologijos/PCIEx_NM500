/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef ONCE_INC_TRANSPORT_PCIE_H_
#define ONCE_INC_TRANSPORT_PCIE_H_

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "sorry, tested only for LITTLE_ENDIAN"
#endif // __BYTE_ORDER__

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NTIA_PCIE_VENDORID   (0x1e51u)
#define NTIA_PCIE_DEVICEID   (0x000fu)
#define NTIA_PCIE_CARD_ID    ((NTIA_PCIE_VENDORID << 16) | NTIA_PCIE_DEVICEID)

#define NTIA_PCIE_MEM_SIZE   (4 * 1024)

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

enum ntpcie_io_error_t
{
  NTPCIE_IO_ERROR_SUCCESS          = 0x00000000u,
  NTPCIE_IO_ERROR_UNKNOWN,
  NTPCIE_IO_ERROR_BAD_DEV_HANDLE,
  NTPCIE_IO_ERROR_DATA_READ,
  NTPCIE_IO_ERROR_DATA_WRITE,
  NTPCIE_IO_ERROR_DATASIZE_MISMATCH,
  NTPCIE_IO_ERROR_ALIGN_MISMATCH,
};

struct pcie_io_handle_t
{
  void* _iox_handle;
  uint32_t _u32x_space;
};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  /**
   * @brief         TODO
   * @details       TODO
   * @param[in]
   * @param[out]
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_init(struct pcie_io_handle_t* const io_handle);

  /**
   * @brief         TODO
   * @details       TODO
   * @param[in]
   * @param[out]
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_deinit(struct pcie_io_handle_t* const io_handle);

  /**
   * @brief         scan PCIe bus and get list of known devices
   * @details       TODO
   * @param[in]
   * @param[out]
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_device_scan(struct pcie_io_handle_t * const io_handle,
                                                  struct nta_pcidev_list_t * const devs_list);

  /**
   * @brief         open PCIe device and construct (internal data structs) device handle
   * @details       TODO
   * @param[in/out] io_handle pointer to device handle instance
   * @param[out]
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_device_open(struct pcie_io_handle_t* const io_handle,
                                                  const uint16_t pci_bus,
                                                  const uint16_t pci_slot);

  /**
   * @brief         close PCIe device and destroy device handle
   * @details       TODO
   * @param[in/out] io_handle pointer to device handle instance
   * @param[in]     pci_bus
   * @param[in]     pci_slot
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_device_close(struct pcie_io_handle_t* const io_handle);

  /**
   * @brief         read data (uint32_t) from PCIe device
   * @details       TODO
   * @param[in]     io_handle pointer to device handle instance
   * @param[in]     offset offset in PCIe address space
   * @param[out]    data pointer to uint32_t variable to store data
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_device_rd32(const struct pcie_io_handle_t* const io_handle,
                                                  const uint32_t offset,
                                                  uint32_t* const data);
  /**
   * @brief         write data (uint32_t) to PCIe device
   * @details       TODO
   * @param[in]     io_handle pointer to device handle instance
   * @param[in]     offset offset in PCIe address space
   * @param[in]     data uint32_t value to store in PCIe device
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_device_wr32(const struct pcie_io_handle_t* const io_handle,
                                                  const uint32_t offset, uint32_t const data);
  /**
   * @brief         read array of data (uint32_t) from PCIe device
   * @details       TODO
   * @param[in]     io_handle pointer to device handle instance
   * @param[in]     offset offset in PCIe address space
   * @param[out]    data pointer to uint32_t array to store data
   * @param[in]     data_length quantity of elements in data array
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_device_mem_rd32(const struct pcie_io_handle_t* const io_handle,
                                                      const uint32_t offset,
                                                      void* const data,
                                                      const uint32_t data_length);
  /**
   * @brief         write array of data (uint32_t) to PCIe device
   * @details       TODO
   * @param[in]     io_handle pointer to device handle instance
   * @param[in]     offset offset in PCIe address space
   * @param[out]    data pointer to uint32_t array to store in PCIe device
   * @param[in]     data_length quantity of elements in data array
   * @return        error code (NTPCIE_IO_ERROR_SUCCESS is OK)
   */
  enum ntpcie_io_error_t ntia_pcie_io_device_mem_wr32(const struct pcie_io_handle_t* const io_handle,
                                                      const uint32_t offset,
                                                      const void* const data,
                                                      const uint32_t data_length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ONCE_INC_TRANSPORT__PCIE_H_
