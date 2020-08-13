set(CMAKE_SYSTEM_NAME         Linux)
set(CMAKE_SYSTEM_PROCESSOR    riscv64)

set(_C_TRIPLET    "riscv64-linux-musl")
set(_C_PREFIX     "/usr/bin/${_C_TRIPLET}-")

set(CMAKE_C_COMPILER          "${_C_PREFIX}gcc")
set(CMAKE_CXX_COMPILER        "${_C_PREFIX}g++")
set(CMAKE_ASM_COMPILER        "${_C_PREFIX}as")

set(CMAKE_C_COMPILER_ID       GNU)
set(CMAKE_CXX_COMPILER_ID     GNU)
set(CMAKE_ASM_COMPILER_ID     GNU)

set(CMAKE_C_COMPILER_FORCED   TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_ASM_COMPILER_FORCED TRUE)