#pragma once

// clang-format off
#include <stdint.h>

#include <windows.h>
#include <guiddef.h>
// clang-format on

// {3F4699BC-22CE-41F7-AF5B-1A9040888247}
DEFINE_GUID(GUID_DEVINTERFACE_NTIA_PCIE, 0x3f4699bc, 0x22ce, 0x41f7, 0xaf, 0x5b, 0x1a, 0x90, 0x40, 0x88, 0x82, 0x47);

#define NTIA_PCIE_BAR0_SIZE (4096)

#define NTIA_PCIE_IOCTL_HDR_REQ_SIZE (2 * sizeof(uint32_t))                               // manual calculated from struct _PCIE_MMIO32_REQ (bellow)
#define NTIA_PCIE_IOCTL_MAX_REQ_SIZE (NTIA_PCIE_BAR0_SIZE - NTIA_PCIE_IOCTL_HDR_REQ_SIZE) // 4KiB (page/bar size) - size_of_header

enum _IOCTL_NTIA_OPCODES
{
    IOCTL_NTIA_PCIE_MMIO_RD32 = 0xC0DE8300u,
    IOCTL_NTIA_PCIE_MMIO_WR32 = 0xC0DE8310u,

    IOCTL_NTIA_PCIE_XFFFFFFFF = 0xFFFFFFFFu
};

typedef struct _PCIE_MMIO32_REQ
{
    uint32_t offset_in_octets;
    uint32_t length_in_octets;
    uint8_t user_data[NTIA_PCIE_IOCTL_MAX_REQ_SIZE];
} PCIE_MMIO32_REQ;
