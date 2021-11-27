cmake_minimum_required(VERSION 3.21)
project(slow)

function(slow_common_options Target)
    set_target_properties(${Target} PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    target_compile_options(${Target} PRIVATE
        "/utf-8"
        "/MP"
    )
    target_compile_definitions(${Target} PUBLIC
        _UNICODE
        UNICODE
    )
endfunction()

add_executable(slow_make_type src/make_type.cpp)
slow_common_options(slow_make_type)

add_library(slow STATIC)
slow_common_options(slow)
target_precompile_headers(slow PRIVATE src/pch.hpp)
target_include_directories(slow PUBLIC
    include
)
target_include_directories(slow PRIVATE
    src
)
target_sources(slow PRIVATE
    src/pch.cpp
    src/slow.cpp
)

add_custom_command(TARGET slow_make_type PRE_BUILD
    COMMAND echo ${CMAKE_BINARY_DIR}/slow
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/slow
    COMMAND $<TARGET_FILE:slow_make_type> -o ${CMAKE_BINARY_DIR}/slow/NativeType.hpp
)
