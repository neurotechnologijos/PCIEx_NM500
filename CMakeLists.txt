CMAKE_MINIMUM_REQUIRED(VERSION 3.9.4 FATAL_ERROR)

enable_language(C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD   11)

set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Set project version
SET(NTIA_API_MAJOR_VERSION 2019)
SET(NTIA_API_MINOR_VERSION 01)
SET(NTIA_API_PATCH_VERSION 00)

project(ntia.api VERSION ${NTIA_API_MAJOR_VERSION}.${NTIA_API_MINOR_VERSION}.${NTIA_API_PATCH_VERSION})

if(NOT NTIA_I_AM_DEVELOPER)
  if(WIN32)
    message(FATAL_ERROR "FATAL_ERROR (temporary): sorry, not yet implemented build on win32 ...")
  endif(WIN32)
endif(NOT NTIA_I_AM_DEVELOPER)

set(EXECUTABLE_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/bin")
if(NOT WIN32)
  set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/lib")
else(NOT WIN32)
  set(LIBRARY_OUTPUT_PATH    "${CMAKE_BINARY_DIR}/bin")
endif(NOT WIN32)

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-DWINVER=0x0601)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif(WIN32)

set(NTIA_INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/external"
  "${CMAKE_SOURCE_DIR}/api"
)

set(COMMON_TOOLS_FILES
  "${CMAKE_SOURCE_DIR}/external/crc32.c"
  "${CMAKE_SOURCE_DIR}/external/hash32djb2.c"
  "${CMAKE_SOURCE_DIR}/external/rand_simple.c"
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

remove_definitions(-DNTIAPCIE_DEBUG)
# disable some runtime integrity checks for (HARDWARE) debug purposes
### add_compile_definitions(NTIAPCIE_DEBUG)

add_subdirectory(driver)
add_subdirectory(src)
add_subdirectory(examples/c)
add_subdirectory(examples/cxx)
