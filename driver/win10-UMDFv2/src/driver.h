#pragma once
// clang-format off
#include <windows.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "queue.h"
// clang-format on

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD ntia_pcie_evt_device_add;
EVT_WDF_OBJECT_CONTEXT_CLEANUP ntia_pcie_evt_driver_context_cleanup;

EXTERN_C_END
