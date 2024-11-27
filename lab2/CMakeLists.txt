cmake_minimum_required(VERSION 3.10)
project(Processes)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(RESULT_DIR "${CMAKE_BINARY_DIR}/bin")
set(LIB_DIR "${CMAKE_BINARY_DIR}/lib")

if(WIN32)
    add_library(process SHARED ${SOURCE_DIR}/libprocess.c)

    set_target_properties(process PROPERTIES
        OUTPUT_NAME process
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        POSITION_INDEPENDENT_CODE ON
    )

    add_executable(main ${SOURCE_DIR}/main.c)

    target_link_libraries(main process)

    set_target_properties(main PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_executable(child ${SOURCE_DIR}/child.c)
    set_target_properties(child PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

else()
    add_library(process SHARED ${SOURCE_DIR}/libprocess.c)

    set_target_properties(process PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}
        POSITION_INDEPENDENT_CODE ON
    )

    add_executable(main ${SOURCE_DIR}/main.c)

    target_link_libraries(main ${LIB_DIR}/libprocess.so)

    set_target_properties(main PROPERTIES
        INSTALL_RPATH ${RESULT_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
    )

    add_executable(child ${SOURCE_DIR}/child.c)
    set_target_properties(child PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
    )

endif()



