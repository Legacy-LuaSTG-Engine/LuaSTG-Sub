
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
    spdlog::spdlog
    utility
    imgui
    imgui_impl_win32
    imgui_impl_dx11
    nlohmann_json
    PlatformAPI
)

add_custom_command(TARGET Setting POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # luastg configuer
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:Setting>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:Setting>/$<TARGET_FILE_NAME:Setting>"            ${CMAKE_SOURCE_DIR}/bin
)
