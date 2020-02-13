/*
 * Copyright (c) 2017-2019 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef _WIN32
#error "sorry, this module for win10 ONLY"
#endif // _WIN32

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
#include <windows.h>
#include <cfgmgr32.h>
#include <strsafe.h>
#include <initguid.h>

#ifndef INITGUID
#define INITGUID
#endif // INITGUID
#include "dev_io_proto.h"
// clang-format on

#include "pcie/transport_pcie.h"
#include "transport_umdfv2.h"

//#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wunused-function"
//#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wunused-const-variable"
//#pragma GCC diagnostic ignored "-Wunused-label"

struct uxio_device_t
{
    struct nta_pcidev_info_t device_bus_address;
    WCHAR device_path[NTIA_MAX_DEVPATH_LENGTH];
    HANDLE _iox_handle;
};

struct uxio_device_list_t
{
    size_t devices_active;
    struct uxio_device_t devices[NTIA_PCIE_MAX_CARDS];
};

static struct uxio_device_list_t uio_dev_list;

/// internal functions
static bool uio_dev_scan_system(struct uxio_device_list_t * const dev_list, GUID const* const umdfv2_iface_guid);
static void uio_dev_handle_reset(struct uxio_device_list_t* const dev_list);
static void uio_dev_handle_close_all(struct uxio_device_list_t* const dev_list);
static size_t uio_dev_handle_find(struct uxio_device_list_t const* const dev_list,
                                  const uint16_t pci_bus,
                                  const uint16_t pci_slot,
                                  const uint16_t pci_func);

static bool uio_dev_rd32(HANDLE sys_dev_handle,
                         const uint32_t offset,
                         void* const data,
                         const uint32_t data_length_octets);

static bool uio_dev_wr32(HANDLE sys_dev_handle,
                         const uint32_t offset,
                         const void* const data,
                         const uint32_t data_length_octets);

/// services public functions
enum ntpcie_io_error_t ntia_pcie_io_init(struct pcie_io_handle_t* const io_handle)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
    if (io_handle == NULL || io_handle->_iox_handle != NULL)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }

    struct uxio_device_list_t * const int_devs_list = &uio_dev_list;
    bool system_scan_result                         = false;

    uio_dev_handle_reset(int_devs_list);
    system_scan_result = uio_dev_scan_system(int_devs_list, &GUID_DEVINTERFACE_NTIA_PCIE);

    io_handle->_u32x_space = NTIA_PCIE_INVALID_SP;
    if (system_scan_result)
    {
        io_handle->_iox_handle = int_devs_list;
        io_result              = NTPCIE_IO_ERROR_SUCCESS;
        goto ret_result;
    }
    else
    {
        io_handle->_iox_handle = NULL;
        io_result              = NTPCIE_IO_ERROR_UNKNOWN;
        goto ret_result;
    }

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
    io_handle->_u32x_space = NTIA_PCIE_INVALID_SP;
    io_result              = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
    return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_scan(struct pcie_io_handle_t* const io_handle, struct nta_pcidev_list_t* const devs_list)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

    if (io_handle == NULL || io_handle->_iox_handle == NULL)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }

    struct uxio_device_list_t const* const int_devs_list = io_handle->_iox_handle;

    uint16_t c_pci_bus, c_pci_slot, c_pci_func;

    devs_list->pci_id_vendor = NTIA_PCIE_VENDORID;
    devs_list->pci_id_device = NTIA_PCIE_DEVICEID;
    devs_list->devs_count    = 0;

    for (size_t ix = 0; ix < int_devs_list->devices_active; ++ix)
    {
        c_pci_bus  = int_devs_list->devices[ix].device_bus_address.bus;
        c_pci_slot = int_devs_list->devices[ix].device_bus_address.slot;
        c_pci_func = int_devs_list->devices[ix].device_bus_address.func;

        devs_list->devices[ix].bus  = c_pci_bus;
        devs_list->devices[ix].slot = c_pci_slot;
        devs_list->devices[ix].func = c_pci_func;
    }
    devs_list->devs_count = int_devs_list->devices_active;
    io_result             = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
    return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_open(struct pcie_io_handle_t* const io_handle,
                                                const uint16_t pci_bus,
                                                const uint16_t pci_slot,
                                                const uint16_t pci_func)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

    if (io_handle == NULL || io_handle->_iox_handle == NULL)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }

    struct uxio_device_list_t * const int_devs_list = io_handle->_iox_handle;
    const size_t device_ix = uio_dev_handle_find(int_devs_list, pci_bus, pci_slot, pci_func);

    if (device_ix == NTIA_PCIE_INVALID_SP || int_devs_list->devices[device_ix]._iox_handle != INVALID_HANDLE_VALUE)
    {
        io_result = NTPCIE_IO_ERROR_UNKNOWN;
        goto ret_result;
    }

    HANDLE sys_dev_handle = INVALID_HANDLE_VALUE;
    sys_dev_handle = CreateFile(int_devs_list->devices[device_ix].device_path,
                                GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (sys_dev_handle == INVALID_HANDLE_VALUE)
    {
        io_result = NTPCIE_IO_ERROR_UNKNOWN;
        goto ret_result;
    }

    int_devs_list->devices[device_ix]._iox_handle = sys_dev_handle;
    io_handle->_u32x_space                        = device_ix;

    io_result = NTPCIE_IO_ERROR_SUCCESS;

ret_result:
    return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_close(struct pcie_io_handle_t* const io_handle)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;
    if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == NTIA_PCIE_INVALID_SP)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }

    struct uxio_device_list_t * const int_devs_list = io_handle->_iox_handle;
    const size_t device_ix                          = io_handle->_u32x_space;

    if (device_ix == NTIA_PCIE_INVALID_SP || int_devs_list->devices[device_ix]._iox_handle == INVALID_HANDLE_VALUE)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }

    CloseHandle(int_devs_list->devices[device_ix]._iox_handle);
    int_devs_list->devices[device_ix]._iox_handle = INVALID_HANDLE_VALUE;

    io_handle->_u32x_space = NTIA_PCIE_INVALID_SP;
    io_result = NTPCIE_IO_ERROR_SUCCESS;
ret_result:
    return io_result;
}

/// IO functions

enum ntpcie_io_error_t ntia_pcie_io_device_rd32(const struct pcie_io_handle_t* const io_handle, const uint32_t offset, uint32_t* const data)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

    if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == NTIA_PCIE_INVALID_SP)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }

    struct uxio_device_list_t const* const int_devs_list = io_handle->_iox_handle;
    const size_t device_ix                               = io_handle->_u32x_space;

    // transfer data
    bool sys_io_result = false;
    sys_io_result = uio_dev_rd32(int_devs_list->devices[device_ix]._iox_handle, offset, data, sizeof(*data));
    io_result = (sys_io_result) ? NTPCIE_IO_ERROR_SUCCESS : NTPCIE_IO_ERROR_DATA_READ;

ret_result:
    return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_wr32(const struct pcie_io_handle_t* const io_handle, const uint32_t offset, const uint32_t data)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

    if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == NTIA_PCIE_INVALID_SP)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }

    struct uxio_device_list_t const* const int_devs_list = io_handle->_iox_handle;
    const size_t device_ix                               = io_handle->_u32x_space;

    // transfer data
    bool sys_io_result = false;
    sys_io_result = uio_dev_wr32(int_devs_list->devices[device_ix]._iox_handle, offset, &data, sizeof(data));
    io_result = (sys_io_result) ? NTPCIE_IO_ERROR_SUCCESS : NTPCIE_IO_ERROR_DATA_WRITE;

ret_result:
    return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_mem_rd32(const struct pcie_io_handle_t* const io_handle,
                                                    const uint32_t offset,
                                                    void* const data,
                                                    const uint32_t data_length_octets)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

    if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == NTIA_PCIE_INVALID_SP)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }
    else if ((data_length_octets & 0x3) != 0)
    {
        io_result = NTPCIE_IO_ERROR_DATASIZE_MISMATCH;
        goto ret_result;
    }

    struct uxio_device_list_t const* const int_devs_list = io_handle->_iox_handle;
    const size_t device_ix                               = io_handle->_u32x_space;

    // transfer data
    bool sys_io_result = false;
    sys_io_result = uio_dev_rd32(int_devs_list->devices[device_ix]._iox_handle, offset, data, data_length_octets);
    io_result = (sys_io_result) ? NTPCIE_IO_ERROR_SUCCESS : NTPCIE_IO_ERROR_DATA_READ;

ret_result:
    return io_result;
}

enum ntpcie_io_error_t ntia_pcie_io_device_mem_wr32(const struct pcie_io_handle_t* const io_handle,
                                                    const uint32_t offset,
                                                    const void* const data,
                                                    const uint32_t data_length_octets)
{
    enum ntpcie_io_error_t io_result = NTPCIE_IO_ERROR_SUCCESS;

    if (io_handle == NULL || io_handle->_iox_handle == NULL || io_handle->_u32x_space == NTIA_PCIE_INVALID_SP)
    {
        io_result = NTPCIE_IO_ERROR_BAD_DEV_HANDLE;
        goto ret_result;
    }
    else if ((data_length_octets & 0x3) != 0)
    {
        io_result = NTPCIE_IO_ERROR_DATASIZE_MISMATCH;
        goto ret_result;
    }

    struct uxio_device_list_t const* const int_devs_list = io_handle->_iox_handle;
    const size_t device_ix                               = io_handle->_u32x_space;

    // transfer data
    bool sys_io_result = false;
    sys_io_result = uio_dev_wr32(int_devs_list->devices[device_ix]._iox_handle, offset, data, data_length_octets);
    io_result = (sys_io_result) ? NTPCIE_IO_ERROR_SUCCESS : NTPCIE_IO_ERROR_DATA_WRITE;

ret_result:
    return io_result;
}

/// internal functions
static size_t uio_dev_handle_find(struct uxio_device_list_t const* const dev_list,
                                  const uint16_t pci_bus,
                                  const uint16_t pci_slot,
                                  const uint16_t pci_func)
{
    size_t device_ix = NTIA_PCIE_INVALID_SP;
    for (size_t ix = 0; ix < dev_list->devices_active; ++ix)
    {
        if (dev_list->devices[ix].device_bus_address.bus == pci_bus
         && dev_list->devices[ix].device_bus_address.slot == pci_slot
         && dev_list->devices[ix].device_bus_address.func == pci_func)
        {
            device_ix = ix;
            goto ret_result;
        }
    }
ret_result:
    return device_ix;
}

static void uio_dev_handle_reset(struct uxio_device_list_t* const dev_list)
{
    memset(dev_list, 0, sizeof(*dev_list));
    dev_list->devices_active = 0;
    for (size_t ix = 0; ix < NTIA_PCIE_MAX_CARDS; ix++)
    {
        dev_list->devices[ix].device_bus_address.bus  = 0xFFFFu;
        dev_list->devices[ix].device_bus_address.slot = 0xFFFFu;
        dev_list->devices[ix].device_bus_address.func = 0x0000u;
        dev_list->devices[ix]._iox_handle             = INVALID_HANDLE_VALUE;
    }
}

static void uio_dev_handle_close_all(struct uxio_device_list_t* const dev_list)
{
    for (size_t ix = 0; ix < NTIA_PCIE_MAX_CARDS; ix++)
    {
        if (dev_list->devices[ix]._iox_handle == INVALID_HANDLE_VALUE)
        {
            continue;
        }
        else
        {
            CloseHandle(dev_list->devices[ix]._iox_handle);
            dev_list->devices[ix]._iox_handle = INVALID_HANDLE_VALUE;
        }
    }
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#endif // __GNUC__ or __clang__

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#endif // __clang__

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif // __GNUC__

static bool uio_dev_scan_system(struct uxio_device_list_t * const dev_list, GUID const* const umdfv2_iface_guid)
{
    CONFIGRET c_result          = CR_SUCCESS;
    PWSTR device_iface_list     = NULL;
    ULONG device_iface_list_len = 0;
    PWSTR next_iface            = NULL;
    HRESULT h_result            = E_FAIL;
    bool ret_status             = false;

    c_result = CM_Get_Device_Interface_List_Size(&device_iface_list_len,
                                                 umdfv2_iface_guid,
                                                 NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    if (CR_SUCCESS != c_result || device_iface_list_len <= 1)
    {
        ret_status = false;
        goto ret_cleanup;
    }

    const size_t device_iface_list_len_bytes = device_iface_list_len * sizeof(*device_iface_list);
    device_iface_list = (PWSTR)malloc(device_iface_list_len_bytes);
    if (device_iface_list == NULL)
    {
        ret_status = false;
        goto ret_cleanup;
    }
    memset(device_iface_list, 0, device_iface_list_len_bytes);

    c_result = CM_Get_Device_Interface_List(umdfv2_iface_guid, NULL, device_iface_list, device_iface_list_len, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    if (CR_SUCCESS != c_result)
    {
        ret_status = false;
        goto ret_cleanup;
    }

    dev_list->devices_active = 0;
    size_t ix;
    for (ix = 0, next_iface = device_iface_list;
         *next_iface != UNICODE_NULL;
         ++ix, next_iface += wcslen(next_iface) + 1)
    {
        h_result = StringCchCopy(dev_list->devices[ix].device_path,
                                 sizeof(dev_list->devices[0].device_path)/sizeof(dev_list->devices[0].device_path[0]),
                                 next_iface);
        if (FAILED(h_result))
        {
            ret_status = false;
            goto ret_cleanup;
        }
        dev_list->devices[ix].device_bus_address.bus  = 0xDEADu;
        dev_list->devices[ix].device_bus_address.slot = (uint16_t)ix;
        dev_list->devices[ix].device_bus_address.func = 0xBEEFu;
        dev_list->devices_active = ix + 1;
    }

    ret_status = true;

ret_cleanup:
    if (device_iface_list != NULL)
    {
        free(device_iface_list);
    }

    return ret_status;
}
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif // __GNUC__ or __clang__

static bool uio_dev_rd32(HANDLE sys_dev_handle,
                         const uint32_t offset,
                         void* const data,
                         const uint32_t data_length_octets)
{
    PCIE_MMIO32_REQ req_tx;
    PCIE_MMIO32_REQ req_rx;
    bool ret_status = false;
    DWORD octets_proc;

    if (data_length_octets < NTIA_PCIE_IOCTL_MAX_REQ_SIZE)
    {
        req_tx.offset_in_octets = offset;
        req_tx.length_in_octets = data_length_octets;
        ret_status              = DeviceIoControl(sys_dev_handle, IOCTL_NTIA_PCIE_MMIO_RD32, &req_tx, sizeof(req_tx), &req_rx, sizeof(req_rx), &octets_proc, NULL);
        memcpy(data, &req_rx.user_data[0], data_length_octets);
    } else
    {
        ret_status = false;
    }
    return ret_status;

}

static bool uio_dev_wr32(HANDLE sys_dev_handle,
                         const uint32_t offset,
                         const void* const data,
                         const uint32_t data_length_octets)
{
    PCIE_MMIO32_REQ req_tx;
    bool ret_status = false;
    DWORD octets_proc = 0;

    if (data_length_octets < NTIA_PCIE_IOCTL_MAX_REQ_SIZE)
    {
        req_tx.offset_in_octets = offset;
        req_tx.length_in_octets = data_length_octets;
        memcpy(&req_tx.user_data[0], data, req_tx.length_in_octets);
        ret_status = DeviceIoControl(sys_dev_handle, IOCTL_NTIA_PCIE_MMIO_WR32, &req_tx, sizeof(req_tx), NULL, 0, &octets_proc, NULL);
    } else
    {
        ret_status = false;
    }
    return ret_status;
}
