set(CMAKE_SYSTEM_NAME         Windows)
set(CMAKE_SYSTEM_PROCESSOR    AMD64)

set(_C_PREFIX     "c:/devtools/msys64/mingw64/bin/")

set(CMAKE_C_COMPILER          "${_C_PREFIX}clang.exe")
set(CMAKE_CXX_COMPILER        "${_C_PREFIX}clang++.exe")
set(CMAKE_ASM_COMPILER        "${_C_PREFIX}as.exe")

set(CMAKE_C_COMPILER_ID       Clang)
set(CMAKE_CXX_COMPILER_ID     Clang)
set(CMAKE_ASM_COMPILER_ID     GNU)

set(CMAKE_C_COMPILER_FORCED   TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_ASM_COMPILER_FORCED TRUE)
