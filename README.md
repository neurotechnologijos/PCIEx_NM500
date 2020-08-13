# NeuroTechnologijos UAB neural boards

## NT Adaptive PCIe card library

![pcie-card](pictures/NT-Adaptive-Neurotechnologijos-114.png)

NT Adaptive PCIe neural boards can recognize static and video images, sounds, various electrical signals, text, data.
Neuromorphic memory NT Adaptive PCIe series control boards have no microcontroller and can be mounted in any PC or other
device with a PCIe connector. NT Adaptive PCIe boards come in three versions - with 4, 8 or 16 neural chips.
Signal recognition process in neuromorphic chips takes place at the hardware level and makes huge acceleration for central processor.

to see [details](https://www.neurotechnologijos.com/en/nt-adaptive-pcie-2/) ...

## Get sources
[GitHub repo](https://github.com/neurotechnologijos/PCIEx_NM500.git)

``` bash
git clone https://github.com/neurotechnologijos/PCIEx_NM500.git ntadaptive
cd ntadaptive
mkdir -p build
cd build
```

 **ATTENTION:** following commands assume that your current directory is **`ntadaptive/build/`**

## Build
Build system - cmake based (i.e. build depends on cmake)

* native build (to run on this host) using GNU GCC
``` bash
bash ../scripts/cmake.native-gcc.sh
```

... or if you prefer use LLVM/Clang

* native build (to run on this host) using LLVM/Clang
``` bash
bash ../scripts/cmake.native-clang.sh
```

* for cross-compile build see [BUILD.md](/docs/BUILD.md)

## Install
``` bash
DESTDIR=/tmp/_install_/ cmake --install . --strip
```

## Setup access mode for PCI(e) resources
[driver/linux/README.md](/driver/linux/README.md)


## Authors
* [NeuroTechnologijos UAB](https://www.neurotechnologijos.com/)
* [Dmitry Pimenov](https://github.com/diamondx131/) (aka <diamond.x131-at-gmail.com>)

## License
[MIT](/LICENSE)

[SPDX note](/LICENSE.note)
