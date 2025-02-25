
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
    beautiful_win32_api
    imgui
    imgui_impl_win32
    imgui_impl_dx11
    nlohmann_json
    PlatformAPI
)

set(LUASTG_SETTING_BUILD_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
add_custom_command(TARGET Setting POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LUASTG_SETTING_BUILD_OUTPUT_DIR}
    
    # luastg configuer
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${LUASTG_SETTING_BUILD_OUTPUT_DIR}/$<TARGET_FILE_NAME:Setting>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:Setting>/$<TARGET_FILE_NAME:Setting>"            ${LUASTG_SETTING_BUILD_OUTPUT_DIR}
)
