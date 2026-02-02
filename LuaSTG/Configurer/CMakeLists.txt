
add_executable(Setting WIN32)
luastg_target_common_options(Setting)
luastg_target_more_warning(Setting)
target_precompile_headers(Setting PRIVATE
    SharedHeaders.h
)
target_include_directories(Setting PRIVATE
    .
)
target_sources(Setting PRIVATE
    SharedHeaders.h
    Main.cpp
    Configurer.manifest
)
target_link_libraries(Setting PRIVATE
    d3d11.lib
    dxgi.lib
    spdlog
    utf8
    win32
    beautiful_win32_api
    imgui
    imgui_impl_win32
    imgui_impl_dx11
    nlohmann_json
)

luastg_target_copy_to_bin_directory(Setting Setting)
