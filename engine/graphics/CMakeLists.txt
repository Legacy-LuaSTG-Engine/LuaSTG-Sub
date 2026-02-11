# Graphics

set(lib_name "Core.Graphics")

file(GLOB_RECURSE lib_src RELATIVE ${CMAKE_CURRENT_LIST_DIR}
    core/*.hpp
    core/*.cpp
    d3d11/*.hpp
    d3d11/*.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

add_library(${lib_name} STATIC)
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
if (LUASTG_GRAPHICS_DEBUG_LAYER_ENABLE)
    target_compile_definitions(${lib_name} PRIVATE LUASTG_GRAPHICS_DEBUG_LAYER_ENABLE)
endif ()
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_precompile_headers(${lib_name} PRIVATE
    d3d11/pch.h
)
target_link_libraries(${lib_name} PUBLIC
    # Core
    Core.ReferenceCounted
    Core.Configuration
    Core.FileSystem
    Core.Logging
    Core.Math
    Core.Image
    win32
    # debug
    TracyAPI
    # util
    utility
    utf8
    simdutf::simdutf
    # win32
    winmm.lib
    imm32.lib
    # dx
    dxguid.lib
    dxgi.lib
    d3d11.lib
    d2d1.lib
    Microsoft.Windows.ImplementationLibrary
    Microsoft::DirectXTexMini
    libqoi
    # math
    xmath
    xxhash
    # text
    Freetype::Freetype
    # model
    tinygltf
    # engine
    Core.Math
    Core.String
    Core.Configuration
    Core.ReferenceCounted
    Core.FileSystem
    Core.Image
    Core.Font
    Core.WindowSystem
    win32
    beautiful_win32_api
)

set_target_properties(${lib_name} PROPERTIES FOLDER engine)
