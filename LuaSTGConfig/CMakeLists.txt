
add_executable(LuaSTGConfig WIN32)
luastg_target_common_options(LuaSTGConfig)
target_precompile_headers(LuaSTGConfig PRIVATE
    pch.h
)
target_include_directories(LuaSTGConfig PRIVATE
    ../platform
    ../platform/platform
)
target_sources(LuaSTGConfig PRIVATE
    LuaSTGConfig.manifest
    pch.h
    main.cpp
    ../platform/platform/HighDPI.hpp
    ../platform/platform/HighDPI.cpp
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
)

add_custom_command(TARGET LuaSTGConfig POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # luastg configuer
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:LuaSTGConfig>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:LuaSTGConfig>/$<TARGET_FILE_NAME:LuaSTGConfig>"            ${CMAKE_SOURCE_DIR}/bin
)
