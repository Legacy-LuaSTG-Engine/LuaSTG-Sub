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

set_target_properties(slow_make_type PROPERTIES FOLDER slow)

set(slow_lib_inc
    include/slow/slow.hpp
    include/slow/Type.hpp
    include/slow/Object.hpp
    include/slow/ByteArray.hpp
    include/slow/Graphics/ShaderCompiler.hpp
    include/slow/Graphics/Format.hpp
    include/slow/Graphics/PipelineState.hpp
    include/slow/Graphics/Resource.hpp
    include/slow/Graphics/Device.hpp
)
set(slow_lib_src
    src/slow/slow.cpp
    src/slow/ByteArray.cpp
    src/slow/Graphics/Device.cpp
    src/slow/Graphics/ShaderCompiler.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/include FILES ${slow_lib_inc})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src     FILES ${slow_lib_src})

add_library(slow STATIC)
slow_common_options(slow)
target_precompile_headers(slow PRIVATE src/pch.hpp)
target_include_directories(slow PUBLIC
    #${CMAKE_BINARY_DIR}
    include
)
target_include_directories(slow PRIVATE
    src
)
target_sources(slow PRIVATE
    src/pch.hpp
    src/pch.cpp
    ${slow_lib_inc}
    ${slow_lib_src}
)
target_link_libraries(slow PUBLIC
    dxguid.lib
    dxgi.lib
    d3d11.lib
    d3dcompiler.lib
    windowscodecs.lib
    DirectXTexMini
)

set_target_properties(slow PROPERTIES FOLDER slow)

#add_custom_command(TARGET slow PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/slow
#    COMMAND $<TARGET_FILE:slow_make_type> -o ${CMAKE_BINARY_DIR}/slow/NativeType.hpp
#)

add_library(lua_slow STATIC)
slow_common_options(lua_slow)
target_include_directories(lua_slow PUBLIC
    binding
)
target_sources(lua_slow PRIVATE
    binding/lua_slow.h
    binding/lua_slow.cpp
)
target_link_libraries(lua_slow PUBLIC
    slow
    luajit
)

set_target_properties(lua_slow PROPERTIES FOLDER slow)

add_executable(slow_test src/test.cpp)
slow_common_options(slow_test)
target_link_libraries(slow_test PRIVATE slow)

set_target_properties(slow_test PROPERTIES FOLDER slow)
