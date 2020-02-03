#pragma once

EXTERN_C_START

typedef struct _QUEUE_CONTEXT
{
    WDFDEVICE device;
} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
ntia_pcie_queue_init(_In_ WDFDEVICE Device);

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL ntia_pcie_evt_io_device_control;
EVT_WDF_IO_QUEUE_IO_STOP ntia_pcie_evt_io_stop;

EXTERN_C_END
