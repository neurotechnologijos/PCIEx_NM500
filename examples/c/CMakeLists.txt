set(EXEC_NAME ntapcie_test_c)

include_directories(${INCLUDE_DIRECTORIES})

set(SOURCE_DIR "./")
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.h)

add_executable(${EXEC_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_compile_definitions(${EXEC_NAME} PRIVATE NTIA_API_STATIC)

target_link_libraries(${EXEC_NAME}
    ntiaPCIe_static
    pthread
)

install(TARGETS ${EXEC_NAME} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
