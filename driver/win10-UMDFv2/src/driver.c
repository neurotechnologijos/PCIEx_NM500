#include "driver.h"

NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = ntia_pcie_evt_driver_context_cleanup;

    WDF_DRIVER_CONFIG_INIT(&config, ntia_pcie_evt_device_add);

    status = WdfDriverCreate(DriverObject, RegistryPath, &attributes, &config, WDF_NO_HANDLE);

    return status;
}

NTSTATUS
ntia_pcie_evt_device_add(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit)
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(Driver);

    status = ntia_pcie_create_device(DeviceInit);

    return status;
}

VOID ntia_pcie_evt_driver_context_cleanup(_In_ WDFOBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
}
