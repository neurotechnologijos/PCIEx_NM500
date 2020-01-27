// clang-format off
#include "driver.h"
#include <WinUser.h>
// clang-format on

NTSTATUS
ntia_pcie_evt_device_prepare_hw(_In_ WDFDEVICE Device, _In_ WDFCMRESLIST ResourcesRaw, _In_ WDFCMRESLIST ResourcesTranslated)
{
    NTSTATUS status                            = STATUS_UNSUCCESSFUL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor = NULL;
    PDEVICE_CONTEXT device_context             = DeviceGetContext(Device);

    UNREFERENCED_PARAMETER(ResourcesRaw);

    PAGED_CODE();

    device_context->paddr     = NULL;
    device_context->paddr_u32 = NULL;
    device_context->map_size  = 0;

    // find resource for BAR0 and map to VA
    size_t resource_list_size = WdfCmResourceListGetCount(ResourcesTranslated);
    for (ULONG i = 0; i < resource_list_size; i++)
    {
        descriptor = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
        // ATT: get BAR0 as resource and stop
        if ((descriptor->Type == CmResourceTypeMemory) && (descriptor->u.Memory.Length == NTIA_PCIE_BAR0_SIZE))
        {
            WdfDeviceMapIoSpace(Device, descriptor->u.Memory.Start, descriptor->u.Memory.Length, MmNonCached, &device_context->paddr);
            device_context->map_size  = descriptor->u.Memory.Length;
            device_context->paddr_u32 = WdfDeviceGetHardwareRegisterMappedAddress(Device, device_context->paddr);

            break;
        }
    }
    if (device_context->map_size == NTIA_PCIE_BAR0_SIZE && device_context->paddr != NULL && device_context->paddr_u32 != NULL)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        device_context->paddr     = NULL;
        device_context->paddr_u32 = NULL;
        device_context->map_size  = 0;
        status                    = STATUS_UNSUCCESSFUL;
    }
    return status;
}

NTSTATUS
ntia_pcie_evt_device_release_hw(_In_ WDFDEVICE Device, _In_ WDFCMRESLIST ResourcesTranslated)
{
    NTSTATUS status                = STATUS_SUCCESS;
    PDEVICE_CONTEXT device_context = NULL;

    UNREFERENCED_PARAMETER(ResourcesTranslated);
    PAGED_CODE();

    device_context = DeviceGetContext(Device);
    if (device_context->paddr != NULL)
    {
        WdfDeviceUnmapIoSpace(Device, device_context->paddr, device_context->map_size);
    }
    device_context->paddr     = NULL;
    device_context->paddr_u32 = NULL;
    device_context->map_size  = 0;
    status                    = STATUS_SUCCESS;
    return status;
}

NTSTATUS
ntia_pcie_create_device(_Inout_ PWDFDEVICE_INIT DeviceInit)
{
    WDF_PNPPOWER_EVENT_CALLBACKS pnp_callbacks;
    WDF_OBJECT_ATTRIBUTES device_attrs;
    WDFDEVICE device;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnp_callbacks);

    pnp_callbacks.EvtDevicePrepareHardware = ntia_pcie_evt_device_prepare_hw;
    pnp_callbacks.EvtDeviceReleaseHardware = ntia_pcie_evt_device_release_hw;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnp_callbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&device_attrs, DEVICE_CONTEXT);
    status = WdfDeviceCreate(&DeviceInit, &device_attrs, &device);

    if (NT_SUCCESS(status))
    {
        status = WdfDeviceCreateDeviceInterface(device, &GUID_DEVINTERFACE_NTIA_PCIE, NULL);

        if (NT_SUCCESS(status))
        {
            status = ntia_pcie_queue_init(device);
        }
    }

    return status;
}
