cmake_minimum_required(VERSION 3.15)

add_library(imgui)
target_compile_options(imgui PUBLIC
    "/utf-8"
    "/MP"
    "/std:c++17"
)
target_compile_definitions(imgui PUBLIC
    _UNICODE
    UNICODE
    #IMGUI_USE_BGRA_PACKED_COLOR
    IMGUI_USE_WCHAR32
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    DLL_IMPORT # freetype
)
target_include_directories(imgui PUBLIC
    ../prebuild/freetype/include
    imgui
    imgui/backends
    imgui/misc/cpp
    imgui/misc/freetype
)
target_sources(imgui PRIVATE
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/misc/cpp/imgui_stdlib.h
    
    imgui/misc/freetype/imgui_freetype.cpp
    imgui/misc/freetype/imgui_freetype.h
    
    imgui/backends/imgui_impl_dx9.cpp
    imgui/backends/imgui_impl_dx9.h
    imgui/backends/imgui_impl_win32ex.cpp
    imgui/backends/imgui_impl_win32ex.h
)
target_link_directories(imgui PUBLIC
    ../prebuild/freetype/lib/${EXT_LIBRARY_TYPE}
)
target_link_libraries(imgui
    d3d9.lib
    freetype.lib
)
