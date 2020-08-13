# Get sources
[GitHub repo](https://github.com/neurotechnologijos/PCIEx_NM500.git)

``` bash
git clone https://github.com/neurotechnologijos/PCIEx_NM500.git ntadaptive
cd ntadaptive
mkdir -p build
cd build
```

 **ATTENTION:** following commands assume that your current directory is **`ntadaptive/build/`**

# Build on GNU/Linux
Build system - cmake based (i.e. build depends on cmake)

``` bash
bash ../scripts/cmake.native-gcc.sh
```

## Cross-compile
platform/compiler profiles placed in [`cmake/platforms/`](/cmake/platforms/) directory
and named according to the template `<compiler_family>.<cpu>-<os_platform>-<libc_family>.cmake`

To cross-build, you must have installed appropriate toolchain for target platform.

contents of [`cmake/platforms/`](/cmake/platforms/)
```
clang.amd64-linux.cmake
clang.amd64-msys2.cmake
gcc.aarch64-linux-gnu.cmake
gcc.aarch64-linux-musl.cmake
gcc.amd64-linux.cmake
gcc.amd64-msys2.cmake
gcc.arm-linux-gnueabihf.cmake
gcc.arm-linux-musleabihf.cmake
gcc.riscv64-linux-gnu.cmake
gcc.riscv64-linux-musl.cmake
gcc.x86_64-w64-mingw32.cmake
```

contents of [`scripts/`](/scripts/)
```
cmake.aarch64-linux-gnu-gcc.sh
cmake.aarch64-linux-musl-gcc.sh
cmake.amd64-linux-clang.sh
cmake.amd64-linux-gcc.sh
cmake.amd64-msys2-clang.sh
cmake.amd64-msys2-gcc.sh
cmake.amd64-windows-gcc.sh
cmake.amd64-windows-msvc.cmd
cmake.arm-linux-gnueabihf-gcc.sh
cmake.arm-linux-musleabihf-gcc.sh
cmake.native-clang.sh
cmake.native-gcc.sh
cmake.riscv64-linux-gnu-gcc.sh
cmake.riscv64-linux-musl-gcc.sh
```

Example:

for cross-build libraries and applications for the GNU/Linux system on AArch64 based CPU
and [musl](https://musl.libc.org/) as standard libc using gcc, execute:

``` bash
bash ../scripts/cmake.aarch64-linux-musl-gcc.sh
```
