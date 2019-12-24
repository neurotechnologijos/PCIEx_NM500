/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef __linux__
#error "sorry, this module for Linux ONLY"
#endif // __linux__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <fcntl.h>

#include "pcie/transport_pcie.h"
#include "transport_sysfs.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wunused-label"

// for access to PCI device memory using mapping to virtual memory sysfs files resourceX
// ("/sys/bus/pci/devices/0000:03:00.0/resource[0-n]")
// default access mode to these files is '0600 root:root' (i.e. only root has access)
// for unprivileged users to access PCI you must set the appropriate permissions
// (for example '0666 root:root' for all users access)
// for process automation see /install/linux

// TODO: mount point of sysfs may be NOT '/sys'
static const char dirbase_name_pci_devices[] = "/sys/bus/pci/devices/";
static const char devmem_sys_fn_template[] = "/sys/bus/pci/devices/0000:%02hx:%02hx.%1hx/%s";

static struct uxio_dev_handle_t uio_dev_handle;

/// internal functions
static uint16_t read_pci_id(const char * const _file_name);
static void uio_dev_handle_reset(struct uxio_dev_handle_t* const uio);
static void uio_dev_handle_close_all(struct uxio_dev_handle_t* const uio);

/// services public functions
enum ntpcie_io_error_t ntia_pcie_io_init(struct pcie_io_handle_t* const io_handle)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  if (io_handle == NULL || io_handle->_iox_handle != NULL)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  }

  uio_dev_handle_reset(&uio_dev_handle);
  io_handle->_iox_handle = &uio_dev_handle;
  io_handle->_u32x_space = 0xFFFFFFFFu;
  io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
  return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_deinit(struct pcie_io_handle_t* const io_handle)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  if (io_handle == NULL || io_handle->_iox_handle == NULL)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  }

  uio_dev_handle_close_all(io_handle->_iox_handle);
  uio_dev_handle_reset(io_handle->_iox_handle);
  io_handle->_iox_handle = NULL;
  io_handle->_u32x_space = 0xFFFFFFFFu;
  io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
  return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_scan(struct pcie_io_handle_t  * const io_handle,
                                                struct nta_pcidev_list_t * const devs_list)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

  DIR *sysfs_dir  = NULL;
  struct dirent *dir_item = NULL;

  uint16_t c_pci_bus, c_pci_slot, c_pci_func, c_pci_vendor, c_pci_device;

  devs_list->pci_id_vendor   = NTIA_PCIE_VENDORID;
  devs_list->pci_id_device   = NTIA_PCIE_DEVICEID;
  devs_list->devs_count      = 0;

  sysfs_dir = opendir(dirbase_name_pci_devices);
  if (sysfs_dir == NULL)
  {
    io_result = NTPCIE_IO_ERROR_UNKNOWN;
    goto ret_result;
  }

  while (((dir_item=readdir(sysfs_dir)) != NULL) && (devs_list->devs_count < NTIA_PCIE_MAX_CARDS))
  {
    uint16_t c_pci_domain;
    size_t num_count;
    num_count = sscanf(dir_item->d_name, "%04hx:%02hx:%02hx.%1hx",
                &c_pci_domain, &c_pci_bus, &c_pci_slot, &c_pci_func);
    if (num_count != 4)
    {
      continue;
    }
    char devmem_sys_fn[256];
    snprintf(devmem_sys_fn, 256, devmem_sys_fn_template, c_pci_bus, c_pci_slot, c_pci_func, "vendor");
    c_pci_vendor = read_pci_id(devmem_sys_fn);
    snprintf(devmem_sys_fn, 256, devmem_sys_fn_template, c_pci_bus, c_pci_slot, c_pci_func, "device");
    c_pci_device = read_pci_id(devmem_sys_fn);
    if (c_pci_vendor != devs_list->pci_id_vendor || c_pci_device != devs_list->pci_id_device)
    {
      continue;
    }
    devs_list->devices[devs_list->devs_count].bus   = c_pci_bus;
    devs_list->devices[devs_list->devs_count].slot  = c_pci_slot;
    devs_list->devices[devs_list->devs_count].func  = c_pci_func;
    devs_list->devs_count++;
  }
  io_result = NTPCIE_IO_ERROR_SUCCESS;

clr_sysfs_dir:
  closedir(sysfs_dir);
  sysfs_dir = NULL;
ret_result:
  return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_open(struct pcie_io_handle_t* const io_handle,
                                                const uint16_t pci_bus,
                                                const uint16_t pci_slot)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  int uio_dev_file_handle = (-1);

  if (io_handle == NULL || io_handle->_iox_handle == NULL)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  }

  struct uxio_dev_handle_t* const uio = io_handle->_iox_handle;

  // used fixed BAR0 only and func=0
  const size_t map_ix = 0;
  const uint16_t pci_func = 0;

  char devmem_sys_fn[256];
  snprintf(devmem_sys_fn, 256, devmem_sys_fn_template, pci_bus, pci_slot, pci_func, "resource0");
  uio_dev_file_handle = open(devmem_sys_fn, O_RDWR | O_SYNC);
  if (uio_dev_file_handle == (-1))
  {
    perror("open device memory file (resourceX) failed");
    io_result = NTPCIE_IO_ERROR_UNKNOWN;
    goto ret_result;
  }

  uio->maps[map_ix].iomem = MAP_FAILED;
  uio->maps[map_ix].size  = NTIA_PCIE_MEM_SIZE;
  uio->maps[map_ix].iomem = mmap(NULL, uio->maps[map_ix].size,
                                 PROT_READ | PROT_WRITE, MAP_SHARED,
                                 uio_dev_file_handle, 0);
  if (uio->maps[map_ix].iomem == MAP_FAILED)
  {
    perror("mmap failed");
    io_result = NTPCIE_IO_ERROR_UNKNOWN;
    goto ret_close_file;
  }

  uio->maps_active++;
  io_handle->_u32x_space = map_ix;
  io_result              = NTPCIE_IO_ERROR_SUCCESS;

ret_close_file:
  close(uio_dev_file_handle);
  uio_dev_file_handle = (-1);
ret_result:
  return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_close(struct pcie_io_handle_t* const io_handle)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
  if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == 0xFFFFFFFFu)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  }

  struct uxio_dev_handle_t* const uio = io_handle->_iox_handle;
  const size_t map_ix                 = io_handle->_u32x_space;

  if (uio->maps[map_ix].iomem == MAP_FAILED)
  {
    io_result = NTPCIE_IO_ERROR_UNKNOWN;
    goto ret_result;
  }

  munmap(uio->maps[map_ix].iomem, uio->maps[map_ix].size);
  uio->maps[map_ix].iomem = MAP_FAILED;
  uio->maps[map_ix].size  = 0;
  io_handle->_u32x_space  = 0xFFFFFFFFu;
  io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
  return io_result;
}

/// IO functions

enum ntpcie_io_error_t ntia_pcie_io_device_rd32(const struct pcie_io_handle_t* const io_handle, const uint32_t offset, uint32_t* const data)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

  if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == 0xFFFFFFFFu)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  }

  const struct uxio_dev_handle_t* const uio   = io_handle->_iox_handle;
  const size_t map_ix                         = io_handle->_u32x_space;
  volatile uint32_t* const pcie_iomem_address = (volatile uint32_t*)uio->maps[map_ix].iomem + (offset >> 2);

  // transfer data
  *data = *pcie_iomem_address;
  io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
  return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_wr32(const struct pcie_io_handle_t* const io_handle, const uint32_t offset, const uint32_t data)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

  if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == 0xFFFFFFFFu)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  }

  const struct uxio_dev_handle_t* const uio   = io_handle->_iox_handle;
  const size_t map_ix                         = io_handle->_u32x_space;
  volatile uint32_t* const pcie_iomem_address = (volatile uint32_t*)uio->maps[map_ix].iomem + (offset >> 2);

  // transfer data
  *pcie_iomem_address = data;
  io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
  return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_mem_rd32(const struct pcie_io_handle_t* const io_handle,
                                                    const uint32_t offset,
                                                    void* const data,
                                                    const uint32_t data_length)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

  if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == 0xFFFFFFFFu)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  } else if ((data_length & 0x3) != 0)
  {
    io_result = NTPCIE_IO_ERROR_DATASIZE_MISMATCH;
    goto ret_result;
  }

  const struct uxio_dev_handle_t* const uio   = io_handle->_iox_handle;
  const size_t map_ix                         = io_handle->_u32x_space;
  volatile const uint32_t* pcie_iomem_address = (volatile const uint32_t*)uio->maps[map_ix].iomem + (offset >> 2);
  uint32_t* data_u32_poiner                   = (uint32_t*)data;
  const uint32_t data_u32_length              = (data_length >> 2);

  // transfer data
  for (size_t ix = 0; ix < data_u32_length; ix++)
  {
    *(data_u32_poiner++) = *(pcie_iomem_address++);
  }
  io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
  return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_mem_wr32(const struct pcie_io_handle_t* const io_handle,
                                                    const uint32_t offset,
                                                    const void* const data,
                                                    const uint32_t data_length)
{
  enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

  if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == 0xFFFFFFFFu)
  {
    io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
    goto ret_result;
  } else if ((data_length & 0x3) != 0)
  {
    io_result = NTPCIE_IO_ERROR_DATASIZE_MISMATCH;
    goto ret_result;
  }

  const struct uxio_dev_handle_t* const uio = io_handle->_iox_handle;
  const size_t map_ix                       = io_handle->_u32x_space;
  volatile uint32_t* pcie_iomem_address     = (uint32_t*)uio->maps[map_ix].iomem + (offset >> 2);
  const uint32_t* data_u32_poiner           = (const uint32_t*)data;
  const uint32_t data_u32_length            = (data_length >> 2);

  // transfer data
  for (size_t ix = 0; ix < data_u32_length; ix++)
  {
    *(pcie_iomem_address++) = *(data_u32_poiner++);
  }
  io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
  return io_result;
}

/// internal functions
static uint16_t read_pci_id(const char * const _file_name)
{
  FILE *info_file = NULL;
  size_t num_count;
  uint16_t pci_id = 0;

  info_file = fopen(_file_name, "r");
  if (info_file == NULL)
  {
    pci_id = 0;
  } else
  {
    num_count = fscanf(info_file, "0x%hx", &pci_id);
    if (num_count != 1)
    {
      pci_id = 0;
    }
  }
  fclose(info_file);
  return pci_id;
}

static void uio_dev_handle_reset(struct uxio_dev_handle_t* const uio)
{
  uio->maps_total          = 0;
  uio->maps_active         = 0;
  for (size_t ix = 0; ix < MAX_UIO_MAPS; ix++)
  {
    uio->maps[ix].iomem = MAP_FAILED;
    uio->maps[ix].size  = 0;
  }
}

static void uio_dev_handle_close_all(struct uxio_dev_handle_t* const uio)
{
  for (size_t ix = 0; ix < MAX_UIO_MAPS; ix++)
  {
    if (uio->maps[ix].iomem == MAP_FAILED)
    {
      continue;
    }
    munmap(uio->maps[ix].iomem, uio->maps[ix].size);
    uio->maps[ix].iomem = MAP_FAILED;
    uio->maps[ix].size  = 0;
  }
}

#pragma GCC diagnostic pop
