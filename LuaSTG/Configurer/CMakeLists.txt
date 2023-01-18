
add_executable(LuaSTGConfig WIN32)
luastg_target_common_options(LuaSTGConfig)
luastg_target_more_warning(LuaSTGConfig)
target_precompile_headers(LuaSTGConfig PRIVATE
    SharedHeaders.h
)
target_include_directories(LuaSTGConfig PRIVATE
    .
)
target_sources(LuaSTGConfig PRIVATE
    SharedHeaders.h
    Main.cpp
    Configurer.manifest
)
target_link_libraries(LuaSTGConfig PRIVATE
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

add_custom_command(TARGET LuaSTGConfig POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # luastg configuer
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:LuaSTGConfig>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:LuaSTGConfig>/$<TARGET_FILE_NAME:LuaSTGConfig>"            ${CMAKE_SOURCE_DIR}/bin
)
