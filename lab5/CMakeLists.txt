cmake_minimum_required(VERSION 3.10)
project(Lab5)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(RESULT_DIR "${CMAKE_BINARY_DIR}/bin")

file(MAKE_DIRECTORY ${SOURCE_DIR})
file(MAKE_DIRECTORY ${RESULT_DIR})
file(MAKE_DIRECTORY ${TMP_DIR})

set(SQLITE3_SRC ${SOURCE_DIR}/sqlite3.c)
set(MAIN_SRC ${SOURCE_DIR}/main.c)
set(SIMULATOR_SRC ${SOURCE_DIR}/simulator.c)
set(TEMP_SRC ${SOURCE_DIR}/temp.c)

add_executable(main ${MAIN_SRC} ${SQLITE3_SRC})
set_target_properties(main PROPERTIES
        OUTPUT_NAME main
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)

add_executable(simulator ${SIMULATOR_SRC})
set_target_properties(simulator PROPERTIES
        OUTPUT_NAME simulator
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)

add_executable(temp.cgi ${TEMP_SRC} ${SQLITE3_SRC})
set_target_properties(temp.cgi PROPERTIES
        OUTPUT_NAME temp.cgi
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)

if(WIN32)
    target_link_libraries(main ws2_32)
endif()
