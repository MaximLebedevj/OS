cmake_minimum_required(VERSION 3.10)
project(Lab3)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(RESULT_DIR "${CMAKE_BINARY_DIR}/bin")

file(MAKE_DIRECTORY ${SOURCE_DIR})
file(MAKE_DIRECTORY ${RESULT_DIR})

add_executable(main ${SOURCE_DIR}/main.c)
set_target_properties(main PROPERTIES
        OUTPUT_NAME main
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)

add_executable(copy1 ${SOURCE_DIR}/copy1.c)
set_target_properties(copy1 PROPERTIES
        OUTPUT_NAME copy1
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)

add_executable(copy2 ${SOURCE_DIR}/copy2.c)
set_target_properties(copy2 PROPERTIES
        OUTPUT_NAME copy2
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)
