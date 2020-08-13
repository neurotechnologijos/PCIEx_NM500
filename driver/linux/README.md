## Setup access mode for PCI(e) resources

On GNU/Linux based systems for access to PCI device memory
library use mapping to virtual memory sysfs files `resource[0-n]`
(e.g. `/sys/bus/pci/devices/0000:03:00.0/resource0`).

Default access mode to these files is '0600 root:root' (i.e. **only** root has access).
For unprivileged users to access PCI you must set the appropriate permissions
(for example '0666 root:root' for all users access)

For process automation see files in [`install/linux`](/install/linux/)


contents of [`install/linux`](/install/linux/):
```
install/
└── linux
    ├── modprobe.d
    │   └── pci-stub.conf
    ├── modules-load.d
    │   └── pci-stub.conf
    └── udev
        └── rules.d
            └── 99-ntia-PCIe.rules
```
