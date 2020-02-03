#include <stdint.h>

#include "driver.h"

NTSTATUS
ntia_pcie_queue_init(_In_ WDFDEVICE Device)
{
    WDFQUEUE queue;
    NTSTATUS status;
    PQUEUE_CONTEXT queue_context;
    WDF_IO_QUEUE_CONFIG queue_config;
    WDF_OBJECT_ATTRIBUTES queue_attrs;

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queue_config, WdfIoQueueDispatchSequential);

    queue_config.DefaultQueue       = TRUE;
    queue_config.EvtIoDeviceControl = ntia_pcie_evt_io_device_control;
    queue_config.EvtIoStop          = ntia_pcie_evt_io_stop;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queue_attrs, QUEUE_CONTEXT);

    status = WdfIoQueueCreate(Device, &queue_config, &queue_attrs, &queue);

    if (NT_SUCCESS(status))
    {
        queue_context         = QueueGetContext(queue);
        queue_context->device = Device;
        goto ret_status;
    }
    else
    {
        goto ret_status;
    }

ret_status:
    return status;
}

void ntia_pcie_evt_io_device_control(_In_ WDFQUEUE Queue,
                                     _In_ WDFREQUEST Request,
                                     _In_ size_t OutputBufferLength,
                                     _In_ size_t InputBufferLength,
                                     _In_ ULONG IoControlCode)
{
    PQUEUE_CONTEXT queue_context   = NULL;
    PDEVICE_CONTEXT device_context = NULL;
    PCIE_MMIO32_REQ* req_inp       = NULL;
    PCIE_MMIO32_REQ* req_out       = NULL;
    size_t req_inp_size            = 0;
    size_t req_out_size            = 0;

    volatile uint32_t* pcix_mem_address = NULL;
    volatile uint32_t* user_mem_address = NULL;
    size_t length_in_u32                = 0;

    NTSTATUS ret_status = STATUS_SUCCESS;
    size_t ret_size     = 0;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    queue_context  = QueueGetContext(Queue);
    device_context = DeviceGetContext(queue_context->device);

    switch (IoControlCode)
    {
        case IOCTL_NTIA_PCIE_MMIO_RD32:
            WdfRequestRetrieveInputBuffer(Request, sizeof(PCIE_MMIO32_REQ), &req_inp, &req_inp_size);
            WdfRequestRetrieveOutputBuffer(Request, sizeof(PCIE_MMIO32_REQ), &req_out, &req_out_size);

            // check buffers size
            if (req_inp_size < sizeof(PCIE_MMIO32_REQ) || req_inp->length_in_octets < sizeof(uint32_t))
            {
                ret_size   = 0;
                ret_status = STATUS_INVALID_BUFFER_SIZE;
                goto ret_result;
            }
            if (((size_t)req_inp->offset_in_octets + req_inp->length_in_octets) > device_context->map_size)
            {
                ret_size   = 0;
                ret_status = STATUS_INVALID_ADDRESS;
                goto ret_result;
            }

            pcix_mem_address = (volatile uint32_t*)(device_context->paddr_u32 + (req_inp->offset_in_octets >> 2));
            user_mem_address = (volatile uint32_t*)req_out->user_data;

            length_in_u32 = req_inp->length_in_octets >> 2;
            for (size_t i = 0; i < length_in_u32; ++i)
            {
                *(user_mem_address++) = *(pcix_mem_address++);
            }

            req_out->offset_in_octets = req_inp->offset_in_octets;
            req_out->length_in_octets = (uint32_t)(length_in_u32 * sizeof(uint32_t));

            ret_size   = NTIA_PCIE_IOCTL_HDR_REQ_SIZE + length_in_u32 * sizeof(uint32_t); // sizeof "header" + data
            ret_status = STATUS_SUCCESS;
            break;

        case IOCTL_NTIA_PCIE_MMIO_WR32:
            WdfRequestRetrieveInputBuffer(Request, sizeof(PCIE_MMIO32_REQ), &req_inp, &req_inp_size);

            // check buffers size
            if (req_inp_size < sizeof(PCIE_MMIO32_REQ) || req_inp->length_in_octets < sizeof(uint32_t))
            {
                ret_size   = 0;
                ret_status = STATUS_INVALID_BUFFER_SIZE;
                goto ret_result;
            }
            if (((size_t)req_inp->offset_in_octets + req_inp->length_in_octets) > device_context->map_size)
            {
                ret_size   = 0;
                ret_status = STATUS_INVALID_ADDRESS;
                goto ret_result;
            }

            pcix_mem_address = (volatile uint32_t*)(device_context->paddr_u32 + (req_inp->offset_in_octets >> 2));
            user_mem_address = (volatile uint32_t*)req_inp->user_data;

            length_in_u32 = req_inp->length_in_octets >> 2;
            for (size_t i = 0; i < length_in_u32; ++i)
            {
                *(pcix_mem_address++) = *(user_mem_address++);
            }

            ret_size   = 0;
            ret_status = STATUS_SUCCESS;
            break;

        default:
            ret_size   = 0;
            ret_status = STATUS_INVALID_PARAMETER;
            break;
    }

ret_result:
    WdfRequestCompleteWithInformation(Request, ret_status, ret_size);
    return;
}

void ntia_pcie_evt_io_stop(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ ULONG ActionFlags)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(ActionFlags);
    return;
}
