set(LIBRARY_NAME_BASE ntiaPCIe)
set(LIB_NAME_SHARED "${LIBRARY_NAME_BASE}_shared")
set(LIB_NAME_STATIC "${LIBRARY_NAME_BASE}_static")

set(SOURCE_FILES
  ./ntapcie_lib.c
  ./ntapcie_int.c
  ./ntapcie_int.h
)

include_directories(./transport/)

set(LL_TRANSPORT_SOURCES_PCIE_WIN_UMDFV2
  "./transport/pcie/win/transport_umdfv2.h"
  "./transport/pcie/win/transport_umdfv2.c"
)
set(LL_TRANSPORT_SOURCES_PCIE_LNX_SYSFS
  "./transport/pcie/lnx/transport_sysfs.h"
  "./transport/pcie/lnx/transport_sysfs.c"
)

set(SOURCE_DIR "./")

if(WIN32)
  set(LL_TRANSPORT_SOURCES
    ${LL_TRANSPORT_SOURCES_PCIE_WIN_UMDFV2}
  )
else(WIN32)
  set(LL_TRANSPORT_SOURCES
    ${LL_TRANSPORT_SOURCES_PCIE_LNX_SYSFS}
  )
endif(WIN32)

add_library(ntpcie_lib OBJECT
  ${PUBLIC_HEADER_FILES}
  ${LL_HEADER_FILES}
  ${LL_TRANSPORT_SOURCES}
  ${HEADER_FILES}
  ${SOURCE_FILES}
  ${COMMON_TOOLS_FILES}
)

if(WIN32 AND $<CMAKE_C_COMPILER_ID:GNU>)
  target_compile_options(ntpcie_lib PUBLIC "-municode")
endif(WIN32 AND $<CMAKE_C_COMPILER_ID:GNU>)

if(WIN32)
  target_compile_definitions(ntpcie_lib PUBLIC "UNICODE" "_UNICODE")
endif(WIN32)

target_sources(ntpcie_lib PRIVATE ${LL_TRANSPORT_SOURCES})

set_target_properties(ntpcie_lib PROPERTIES POSITION_INDEPENDENT_CODE 1)

add_library(${LIB_NAME_STATIC} STATIC
  $<TARGET_OBJECTS:ntpcie_lib>
)

add_library(${LIB_NAME_SHARED} SHARED
  $<TARGET_OBJECTS:ntpcie_lib>
  $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/version.rc>
)

target_compile_definitions(${LIB_NAME_SHARED} PRIVATE NTIA_API_DLL NTIA_API_DLL_EXPORTS)

target_link_libraries(${LIB_NAME_STATIC}
  $<$<PLATFORM_ID:Windows>:${WIN32_LIBRARIES}>
)

target_link_libraries(${LIB_NAME_SHARED}
  $<$<PLATFORM_ID:Windows>:${WIN32_LIBRARIES}>
)

install(TARGETS ${LIB_NAME_STATIC} DESTINATION ${LIBRARY_OUTPUT_PATH})
install(TARGETS ${LIB_NAME_SHARED} DESTINATION ${LIBRARY_OUTPUT_PATH})
