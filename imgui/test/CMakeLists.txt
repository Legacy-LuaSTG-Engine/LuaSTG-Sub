cmake_minimum_required(VERSION 3.15)

set(IMGUI_TEST_ROOT ../imgui)
set(IMGUI_TEST_NAME imgui_test)

add_executable(${IMGUI_TEST_NAME})

target_compile_options(${IMGUI_TEST_NAME} PUBLIC
    "/utf-8"
    "/MP"
    "/std:c++17"
)

target_compile_definitions(${IMGUI_TEST_NAME} PRIVATE
    UNICODE
    _UNICODE
    #IMGUI_USE_BGRA_PACKED_COLOR
    #IMGUI_USE_WCHAR32
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    #DLL_IMPORT # freetype
)

target_include_directories(${IMGUI_TEST_NAME} PRIVATE
    ../../prebuild/freetype/include
    ${IMGUI_TEST_ROOT}
    ${IMGUI_TEST_ROOT}/backends
    ${IMGUI_TEST_ROOT}/misc/cpp
    ${IMGUI_TEST_ROOT}/misc/freetype
    ${IMGUI_TEST_ROOT}/examples/example_win32_workingthread_directx11
)

target_sources(${IMGUI_TEST_NAME} PRIVATE
    ${IMGUI_TEST_ROOT}/imconfig.h
    ${IMGUI_TEST_ROOT}/imgui_demo.cpp
    ${IMGUI_TEST_ROOT}/imgui_draw.cpp
    ${IMGUI_TEST_ROOT}/imgui_internal.h
    ${IMGUI_TEST_ROOT}/imgui_tables.cpp
    ${IMGUI_TEST_ROOT}/imgui_widgets.cpp
    ${IMGUI_TEST_ROOT}/imgui.cpp
    ${IMGUI_TEST_ROOT}/imgui.h
    
    ${IMGUI_TEST_ROOT}/imstb_rectpack.h
    ${IMGUI_TEST_ROOT}/imstb_textedit.h
    ${IMGUI_TEST_ROOT}/imstb_truetype.h
    
    ${IMGUI_TEST_ROOT}/misc/cpp/imgui_stdlib.cpp
    ${IMGUI_TEST_ROOT}/misc/cpp/imgui_stdlib.h
    
    ${IMGUI_TEST_ROOT}/misc/freetype/imgui_freetype.cpp
    ${IMGUI_TEST_ROOT}/misc/freetype/imgui_freetype.h
    
    ${IMGUI_TEST_ROOT}/backends/imgui_impl_dx11.cpp
    ${IMGUI_TEST_ROOT}/backends/imgui_impl_dx11.h
    ${IMGUI_TEST_ROOT}/backends/imgui_impl_win32ex.cpp
    ${IMGUI_TEST_ROOT}/backends/imgui_impl_win32ex.h
    
    ${IMGUI_TEST_ROOT}/examples/example_win32ex_directx11/main.cpp
)

target_link_directories(${IMGUI_TEST_NAME} PRIVATE
    ../../prebuild/freetype/lib
)

target_link_libraries(${IMGUI_TEST_NAME} PRIVATE
    d3d11.lib
    d3dcompiler.lib
    freetypestatic.lib
)
