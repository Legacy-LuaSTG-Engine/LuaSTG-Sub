cmake_minimum_required(VERSION 3.16)
project(slow)

# slow framework

set(SLOW_LIB_NAME slow)

add_library(${SLOW_LIB_NAME})

target_compile_options(${SLOW_LIB_NAME} PRIVATE
    "/std:c++17"
    "/utf-8"
    "/MP"
)

target_compile_definitions(${SLOW_LIB_NAME} PRIVATE
    UNICODE
    _UNICODE
)

target_include_directories(${SLOW_LIB_NAME} PUBLIC
    ./include
)
target_include_directories(${SLOW_LIB_NAME} PRIVATE
    ./src
)

target_sources(${SLOW_LIB_NAME} PRIVATE
    include/slow/Object.hpp
    include/slow/ByteArray.hpp
)

target_link_libraries(${SLOW_LIB_NAME} PUBLIC
    zlib
    zip
)

# test

#add_executable(test test/main.cpp)
#
#target_compile_options(test PRIVATE
#    "/std:c++17"
#    "/utf-8"
#)
#
#target_compile_definitions(test PRIVATE
#    UNICODE
#    _UNICODE
#)
#
#target_link_libraries(test
#    ${SLOW_LIB_NAME}
#)
