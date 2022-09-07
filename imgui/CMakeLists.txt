# imgui

add_library(imgui STATIC)
luastg_target_common_options(imgui)
target_compile_definitions(imgui PUBLIC
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_DISABLE_OBSOLETE_KEYIO
    #IMGUI_USE_BGRA_PACKED_COLOR
    IMGUI_USE_WCHAR32
    IMGUI_ENABLE_FREETYPE
)
target_include_directories(imgui PUBLIC
    imgui
    imgui/misc/cpp
    imgui/misc/freetype
)
target_sources(imgui PRIVATE
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    
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
)
target_link_libraries(imgui PUBLIC
    freetype
)

set_target_properties(imgui PROPERTIES FOLDER external)

# implot

add_library(implot STATIC)
luastg_target_common_options(implot)
target_compile_definitions(implot PUBLIC
    IMPLOT_DISABLE_OBSOLETE_FUNCTIONS
)
target_include_directories(implot PUBLIC
    implot
)
target_sources(implot PRIVATE
    implot/implot_demo.cpp
    implot/implot_internal.h
    implot/implot_items.cpp
    implot/implot.cpp
    implot/implot.h
)
target_link_libraries(implot PUBLIC
    imgui
)

set_target_properties(implot PROPERTIES FOLDER external)

# imgui backend dx11

add_library(imgui_impl_dx11 STATIC)
luastg_target_common_options(imgui_impl_dx11)
target_include_directories(imgui_impl_dx11 PUBLIC
    imgui/backends
)
target_sources(imgui_impl_dx11 PRIVATE
    imgui/backends/imgui_impl_dx11.h
    #imgui/backends/imgui_impl_dx11.cpp
    backend/imgui_impl_dx11.cpp
)
target_link_libraries(imgui_impl_dx11 PUBLIC
    imgui
)
set_target_properties(imgui_impl_dx11 PROPERTIES FOLDER external)

# imgui backend win32ex

add_library(imgui_impl_win32ex STATIC)
luastg_target_common_options(imgui_impl_win32ex)
luastg_target_more_warning(imgui_impl_win32ex)
target_include_directories(imgui_impl_win32ex PUBLIC
    backend
)
target_sources(imgui_impl_win32ex PRIVATE
    backend/imgui_impl_win32ex.cpp
    backend/imgui_impl_win32ex.h
)
target_link_libraries(imgui_impl_win32ex PUBLIC
    imgui
)
set_target_properties(imgui_impl_win32ex PROPERTIES FOLDER external)

# imgui backend win32

add_library(imgui_impl_win32 STATIC)
luastg_target_common_options(imgui_impl_win32)
target_include_directories(imgui_impl_win32 PUBLIC
    imgui/backends
)
target_sources(imgui_impl_win32 PRIVATE
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_win32.h
)
target_link_libraries(imgui_impl_win32 PUBLIC
    imgui
)
set_target_properties(imgui_impl_win32 PROPERTIES FOLDER external)

# imgui lua binding

add_library(lua_imgui STATIC)
luastg_target_common_options(lua_imgui)
luastg_target_more_warning(lua_imgui)
target_include_directories(lua_imgui PUBLIC
    binding/include
)
target_include_directories(lua_imgui PRIVATE
    binding/src
)
target_sources(lua_imgui PRIVATE
    binding/include/lua_imgui.hpp
    binding/include/lua_imgui_type.hpp
    
    binding/src/lua_imgui_common.hpp
    binding/src/lua_imgui_enum.hpp
    binding/src/lua_imgui_enum.cpp
    binding/src/lua_imgui_hash.cpp
    binding/src/lua_imgui_hash.hpp
    binding/src/lua_imgui_ImGui.hpp
    binding/src/lua_imgui_ImGui_ColorX.inl
    binding/src/lua_imgui_ImGui_DragX.inl
    binding/src/lua_imgui_ImGui_InputX.inl
    binding/src/lua_imgui_ImGui_PlotX.inl
    binding/src/lua_imgui_ImGui_register.inl
    binding/src/lua_imgui_ImGui_SliderX.inl
    binding/src/lua_imgui_ImGui.cpp
    binding/src/lua_imgui_ImGuiTextBuffer.cpp
    binding/src/lua_imgui_type.cpp
    binding/src/lua_imgui.cpp
)
target_link_libraries(lua_imgui PUBLIC
    imgui
    lua51static
)

set_target_properties(lua_imgui PROPERTIES FOLDER lualib)
