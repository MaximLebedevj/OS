cmake_minimum_required(VERSION 3.10)
project(Lab4)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(RESULT_DIR "${CMAKE_BINARY_DIR}/bin")
set(TMP_DIR "${RESULT_DIR}/tmp")

file(MAKE_DIRECTORY ${SOURCE_DIR})
file(MAKE_DIRECTORY ${RESULT_DIR})
file(MAKE_DIRECTORY ${TMP_DIR})

add_executable(main ${SOURCE_DIR}/main.c)
set_target_properties(main PROPERTIES
        OUTPUT_NAME main
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)

add_executable(simulator ${SOURCE_DIR}/simulator.c)
set_target_properties(simulator PROPERTIES
        OUTPUT_NAME simulator
        RUNTIME_OUTPUT_DIRECTORY ${RESULT_DIR}
)

file(WRITE ${TMP_DIR}/tmp.txt "")
file(WRITE ${RESULT_DIR}/log.txt "")
file(WRITE ${RESULT_DIR}/log_hour.txt "")
file(WRITE ${RESULT_DIR}/log_day.txt "")
