# LuaSTG

include(Custom.cmake OPTIONAL)
if(NOT DEFINED LUASTG_RESDIR)
    message("LuaSTG will using default config, built-in resources and source codes")
    set(LUASTG_RESDIR res)
else()
    message("LuaSTG will using custom config, built-in resources and source codes: " ${LUASTG_RESDIR})
endif()

# LuaSTG Core && Backend

add_library(LuaSTGCore STATIC)
luastg_target_common_options(LuaSTGCore)
target_precompile_headers(LuaSTGCore PRIVATE
    Backend/framework.hpp
)
target_include_directories(LuaSTGCore PUBLIC
    .
)

set(LuaSTGCore_SRC
    Core/Renderer.hpp
    Backend/framework.hpp
    Backend/framework.cpp
    Backend/Shader.hpp
    Backend/Renderer.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${LuaSTGCore_SRC})
target_sources(LuaSTGCore PRIVATE
    ${LuaSTGCore_SRC}
)

target_link_libraries(LuaSTGCore PUBLIC
    spdlog::spdlog
    fancylib
    fancy2dlib
    d3dcompiler.lib
    dxguid.lib
    dxgi.lib
    d3d11.lib
)

# LuaSTG Engine

add_executable(LuaSTG WIN32)
luastg_target_common_options(LuaSTG)

target_precompile_headers(LuaSTG PRIVATE
    pch/pch.h # common header
)
target_include_directories(LuaSTG PRIVATE
    .
    ..
    src
    ${LUASTG_RESDIR}
    src/Resource
    src/LuaWrapper
)

target_compile_definitions(LuaSTG PRIVATE LDEVVERSION)

aux_source_directory(${LUASTG_RESDIR} LUASTG_RES)

include(Sources.cmake)
target_sources(LuaSTG PRIVATE
    ${LUASTG_ENGINE_SOURCES_COMMON}
    ${LUASTG_ENGINE_SOURCES}
    ${LUASTG_LUA_EXTENSION_SOURCES}
    ${LUASTG_RES}
    ${LUASTG_RESDIR}/resource.rc
    ${LUASTG_RESDIR}/app.ico
)

target_link_libraries(LuaSTG PRIVATE
    spdlog::spdlog
    utility
    zip
    MINIZIP::minizip
    fancylib
    fancy2dlib
    DirectXTexMini
    LuaSTGCore
    xmath
    luajit
    imgui
    lua_cjson
    lua_filesystem
    #lua_xlsx_csv
    lua_imgui
    imgui_impl_win32ex
    imgui_impl_dx11
    lua_steam_api
    nlohmann_json
)

add_custom_command(TARGET LuaSTG POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # DirectX family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        ${CMAKE_SOURCE_DIR}/bin
    
    # lua family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:luajit>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            ${CMAKE_SOURCE_DIR}/bin
    
    # luastg engine
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:LuaSTG>"            ${CMAKE_SOURCE_DIR}/bin
)
