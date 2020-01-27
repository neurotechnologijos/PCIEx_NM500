CMAKE_MINIMUM_REQUIRED(VERSION 3.9.4 FATAL_ERROR)

enable_language(C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD   11)

set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Set project version
SET(NTIA_API_MAJOR_VERSION 2020)
SET(NTIA_API_MINOR_VERSION 02)
SET(NTIA_API_PATCH_VERSION 00)

project(ntia.api VERSION ${NTIA_API_MAJOR_VERSION}.${NTIA_API_MINOR_VERSION}.${NTIA_API_PATCH_VERSION})

set(NTIA_I_AM_DEVELOPER true)
# disable some runtime integrity checks for (HARDWARE) debug purposes
remove_definitions(-DNTIAPCIE_DEBUG)
### add_compile_definitions(NTIAPCIE_DEBUG)

message("** PLATFORM_ID           = " ${CMAKE_SYSTEM_NAME})
message("** CMAKE_C_COMPILER_ID   = " ${CMAKE_C_COMPILER_ID})
message("** CMAKE_CXX_COMPILER_ID = " ${CMAKE_CXX_COMPILER_ID})

set(EXECUTABLE_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/bin")
if(NOT WIN32)
  set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/lib")
else(NOT WIN32)
  set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/bin")
endif(NOT WIN32)

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0A00)
  add_definitions(-DWINVER=0x0A00)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  include_directories("${CMAKE_SOURCE_DIR}/driver/win10-UMDFv2/api")
  set(WIN32_LIBRARIES
      "cfgmgr32"
  )
elseif(LINUX)
    set(LINUX_LIBRARIES
        pthread
    )
endif(WIN32)

set(NTIA_INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/external"
  "${CMAKE_SOURCE_DIR}/api"
)

set(COMMON_TOOLS_FILES
  "${CMAKE_SOURCE_DIR}/external/crc32.c"
  "${CMAKE_SOURCE_DIR}/external/hash32djb2.c"
  "${CMAKE_SOURCE_DIR}/external/rand_simple.c"
  "${CMAKE_SOURCE_DIR}/external/getCPUtime.c"
  )

set(PUBLIC_HEADER_FILES
  "${CMAKE_SOURCE_DIR}/api/ntia_shared_defs.h"
  "${CMAKE_SOURCE_DIR}/api/ntia_api.h"
  "${CMAKE_SOURCE_DIR}/api/ntia_api_data_types.h"
)

set(LL_HEADER_FILES
  "${CMAKE_SOURCE_DIR}/api/ntia_api_ll.h"
  "${CMAKE_SOURCE_DIR}/api/ntia_api_data_types_ll.h"
)

include_directories(${NTIA_INCLUDE_DIRECTORIES})

add_subdirectory(driver)
add_subdirectory(src)
add_subdirectory(examples/c)
add_subdirectory(examples/cxx)
