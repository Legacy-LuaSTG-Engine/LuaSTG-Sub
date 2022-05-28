# LuaSTG configurer

add_subdirectory(configurer)

# LuaSTG

if(NOT DEFINED LUASTG_RESDIR)
    message("LuaSTG will using default config, built-in resources and source codes")
    set(LUASTG_RESDIR res)
else()
    message("LuaSTG will using custom config, built-in resources and source codes: " ${LUASTG_RESDIR})
endif()

# LuaSTG Core && Backend

add_library(LuaSTGCore STATIC)
luastg_target_common_options(LuaSTGCore)
luastg_target_more_warning(LuaSTGCore)
target_precompile_headers(LuaSTGCore PRIVATE
    Backend/framework.hpp
)
target_include_directories(LuaSTGCore PUBLIC
    .
)

set(LuaSTGCore_SRC
    Core/Type.hpp
    Core/Renderer.hpp

    Core/i18n.hpp
    Core/i18n.cpp

    Core/Object.hpp
    Core/Graphics/Window.hpp
    Core/Graphics/Window_Win32.hpp
    Core/Graphics/Window_Win32.cpp
    Core/Graphics/Format.hpp
    Core/Graphics/Format_D3D11.hpp
    Core/Graphics/Device.hpp
    Core/Graphics/Device_D3D11.hpp
    Core/Graphics/Device_D3D11.cpp
    Core/Graphics/SwapChain.hpp
    Core/Graphics/SwapChain_D3D11.hpp
    Core/Graphics/SwapChain_D3D11.cpp
    Core/ApplicationModel.hpp
    Core/ApplicationModel_Win32.hpp
    Core/ApplicationModel_Win32.cpp

    Backend/framework.hpp
    Backend/framework.cpp
    Backend/Shader.hpp
    Backend/Renderer.cpp
    Backend/ModelShader.hpp
    Backend/ModelShader.cpp
    Backend/Model.hpp
    Backend/Model.cpp
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
    tinygltf
    tracy
)

target_link_libraries(fancy2dlib PUBLIC
    LuaSTGCore
)

# LuaSTG Engine

add_executable(LuaSTG WIN32)
luastg_target_common_options(LuaSTG)
luastg_target_more_warning(LuaSTG)

target_precompile_headers(LuaSTG PRIVATE
    pch/pch.h # common header
)
target_include_directories(LuaSTG PRIVATE
    .
    ..
    src
    src/Resource
    src/LuaWrapper
)

target_compile_definitions(LuaSTG PRIVATE LDEVVERSION)

include(Sources.cmake)
target_sources(LuaSTG PRIVATE
    ${LUASTG_ENGINE_SOURCES}
    ${LUASTG_LUA_EXTENSION_SOURCES}
)
if(EXISTS ${LUASTG_RESDIR}/Custom.cmake)
    message("LuaSTG will using " ${LUASTG_RESDIR}/Custom.cmake)
    include(${LUASTG_RESDIR}/Custom.cmake)
elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
    message("LuaSTG will using " ${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
else()
    target_include_directories(LuaSTG PRIVATE
        ${LUASTG_RESDIR}
    )
    aux_source_directory(${LUASTG_RESDIR} LUASTG_RES)
    target_sources(LuaSTG PRIVATE
        ${LUASTG_RES}
        ${LUASTG_RESDIR}/resource.rc
        ${LUASTG_RESDIR}/app.ico
    )
endif()

target_link_libraries(LuaSTG PRIVATE
    spdlog::spdlog
    utility
    platform
    MINIZIP::minizip
    fancylib
    fancy2dlib
    DirectXTexMini
    DirectX::ToolKitMini
    LuaSTGCore
    xmath
    lua51static
    imgui
    implot
    lua_cjson
    lua_filesystem
    #lua_xlsx_csv
    lua_imgui
    imgui_impl_win32ex
    imgui_impl_dx11
    lua_steam_api
    nlohmann_json
    tracy
)

add_custom_command(TARGET LuaSTG POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # DirectX family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        ${CMAKE_SOURCE_DIR}/bin
    
    # lua family
    
    #COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:luajit>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            "$<TARGET_FILE_DIR:LuaSTG>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            ${CMAKE_SOURCE_DIR}/bin
    
    # luastg engine
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:LuaSTG>"            ${CMAKE_SOURCE_DIR}/bin
)
