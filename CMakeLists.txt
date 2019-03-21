cmake_minimum_required(VERSION 3.0)

add_executable(persistence persistence.cpp)
target_compile_options(persistence PRIVATE "-std=c++14")
target_include_directories(persistence PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/gmp-install/include )
target_link_directories(persistence PRIVATE ${CMAKE_SOURCE_DIR}/gmp-install/lib)
target_link_libraries(persistence PRIVATE gmp)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(persistence_coro persistence_coro.cpp)
target_compile_options(persistence_coro PRIVATE "-O3" "-std=c++2a" "-fcoroutines-ts" "-stdlib=libc++")
#target_include_directories(persistence_coro PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/gmp-install/include )
target_link_directories(persistence_coro PRIVATE ${CMAKE_SOURCE_DIR}/gmp-install/lib)
target_link_libraries(persistence_coro PRIVATE gmp ${CONAN_LIBS})
