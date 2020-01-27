#pragma once

#include <stdint.h>

#include "dev_io_proto.h"

EXTERN_C_START

typedef struct _DEVICE_CONTEXT
{
    PVOID paddr;
    uint32_t* paddr_u32;
    size_t map_size;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

NTSTATUS
ntia_pcie_create_device(_Inout_ PWDFDEVICE_INIT DeviceInit);

EXTERN_C_END
