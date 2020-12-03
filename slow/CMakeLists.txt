cmake_minimum_required(VERSION 3.15)
project(slow)

#set(EXT_LIBRARY_TYPE Debug)

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
    ZIP_STATIC
)

target_include_directories(${SLOW_LIB_NAME} PUBLIC
    ./include
)
target_include_directories(${SLOW_LIB_NAME} PRIVATE
    ./src
    ../prebuild/zlib/include
    ../prebuild/libzip/include
)

aux_source_directory(./src LIB_SRC)
target_sources(${SLOW_LIB_NAME} PRIVATE
    ${LIB_SRC}
)

target_link_directories(${SLOW_LIB_NAME} PUBLIC
    ../prebuild/zlib/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libzip/lib/${EXT_LIBRARY_TYPE}
)

target_link_libraries(${SLOW_LIB_NAME} PRIVATE
    zlibstatic.lib
    zipstatic.lib
)

# test

add_executable(test test/main.cpp)

target_compile_options(test PRIVATE
    "/std:c++17"
    "/utf-8"
)

target_compile_definitions(test PRIVATE
    UNICODE
    _UNICODE
)

target_link_libraries(test
    ${SLOW_LIB_NAME}
)
