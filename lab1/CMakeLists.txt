cmake_minimum_required(VERSION 3.15)
project(HelloWorld)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

include_directories(src)

set(SOURCES src/hello.c)

add_executable(main ${SOURCES})

