on GNU/Linux based systems for access to PCI device memory using mapping to virtual memory sysfs files resourceX
("/sys/bus/pci/devices/0000:03:00.0/resource[0-n]")
default access mode to these files is '0600 root:root' (i.e. only root has access)
for unprivileged users to access PCI you must set the appropriate permissions
(for example '0666 root:root' for all users access)

for process automation see /install/linux
