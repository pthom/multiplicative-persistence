cmake_minimum_required(VERSION 3.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(persistence_naive persistence_naive.cpp)
target_compile_options(persistence_naive PRIVATE "-std=c++14")
target_link_libraries(persistence_naive PRIVATE pthread)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(ALGO_USE "COROUTINES" CACHE STRING "Select algorithm mode : vector / coroutines / ranges")
set_property(CACHE ALGO_USE PROPERTY STRINGS VECTORS COROUTINES RANGES)

add_executable(persistence persistence.cpp)
target_compile_options(persistence PRIVATE "-O3" "-std=c++2a" "-fcoroutines-ts" "-stdlib=libc++")
target_link_libraries(persistence PRIVATE gmp ${CONAN_LIBS} pthread c++)
if ("${ALGO_USE}" STREQUAL "VECTORS")
    target_compile_definitions(persistence PRIVATE "ALGO_USE_VECTOR")
elseif("${ALGO_USE}" STREQUAL "COROUTINES")
    target_compile_definitions(persistence PRIVATE "ALGO_USE_COROUTINES")
elseif("${ALGO_USE}" STREQUAL "RANGES")
    target_compile_definitions(persistence PRIVATE "ALGO_USE_RANGES")
else()
    message(FATAL_ERROR "Incorrect value for ALGO_USE: ${ALGO_USE}")
endif()
message(STATUS "Compiling with -DALGO_USE=${ALGO_USE}")
