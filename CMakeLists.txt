cmake_minimum_required(VERSION 3.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(persistence_naive persistence_naive.cpp)
target_compile_options(persistence_naive PRIVATE "-std=c++14")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(persistence_coro persistence_coro.cpp)
target_compile_options(persistence_coro PRIVATE "-O3" "-std=c++2a" "-fcoroutines-ts" "-stdlib=libc++")
target_link_directories(persistence_coro PRIVATE ${CMAKE_SOURCE_DIR}/gmp-install/lib)
target_link_libraries(persistence_coro PRIVATE gmp ${CONAN_LIBS})
