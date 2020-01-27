set(EXEC_NAME ntapcie_test_cxx)

include_directories(${INCLUDE_DIRECTORIES})

set(SOURCE_DIR "./")
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.hpp)

add_executable(${EXEC_NAME}
    ${COMMON_TOOLS_FILES}
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_compile_definitions(${EXEC_NAME} PRIVATE NTIA_API_STATIC)

target_link_libraries(${EXEC_NAME}
    ntiaPCIe_static
    $<$<PLATFORM_ID:Linux>:${LINUX_LIBRARIES}>
    $<$<PLATFORM_ID:Windows>:${WIN32_LIBRARIES}>
)

install(TARGETS ${EXEC_NAME} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
