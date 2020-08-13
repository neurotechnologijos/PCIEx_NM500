set(CMAKE_SYSTEM_NAME         Linux)
set(CMAKE_SYSTEM_PROCESSOR    x86_64)

set(_C_TRIPLET    "x86_64-pc-linux-gnu")
set(_C_PREFIX     "/usr/bin/${_C_TRIPLET}-")

set(CMAKE_C_COMPILER          "${_C_PREFIX}gcc")
set(CMAKE_CXX_COMPILER        "${_C_PREFIX}g++")
set(CMAKE_ASM_COMPILER        "${_C_PREFIX}as")
set(CMAKE_AR                  "${_C_PREFIX}gcc-ar")

set(CMAKE_C_COMPILER_ID       GNU)
set(CMAKE_CXX_COMPILER_ID     GNU)
set(CMAKE_ASM_COMPILER_ID     GNU)

set(CMAKE_C_COMPILER_FORCED   TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_ASM_COMPILER_FORCED TRUE)
