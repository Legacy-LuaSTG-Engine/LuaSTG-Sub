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

aux_source_directory(../Common COMMON_SRC)
aux_source_directory(${LUASTG_RESDIR} LUASTG_RES)

include(Sources.cmake)
target_sources(LuaSTG PRIVATE
    ${COMMON_SRC}
    ${LUASTG_ENGINE_SOURCES}
    ${LUASTG_RES}
    ../xinput/xinput.hpp
    ../xinput/xinput.cpp
    ../xinput/lua_xinput.hpp
    ../xinput/lua_xinput.cpp
    ${LUASTG_RESDIR}/resource.rc
    ${LUASTG_RESDIR}/app.ico
)

target_link_libraries(LuaSTG PRIVATE
    spdlog::spdlog
    fancylib
    fancy2dlib
    eyes2d
    DirectXTex0
    LuaSTGCore
    xmath
    luajit
    imgui
    lua_cjson
    lua_fs
    lua_xlsx_csv
    lua_imgui
    lua_slow
    lua_steam_api
)

add_custom_command(TARGET LuaSTG POST_BUILD
    # output dir
    
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    
    # z family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:zlib>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zlib>/$<TARGET_FILE_NAME:zlib>"                "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zlib>/$<TARGET_FILE_NAME:zlib>"                ${CMAKE_SOURCE_DIR}/bin
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:zip>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zip>/$<TARGET_FILE_NAME:zip>"                  "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:zip>/$<TARGET_FILE_NAME:zip>"                  ${CMAKE_SOURCE_DIR}/bin
    
    # xiph family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:ogg>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:ogg>/$<TARGET_FILE_NAME:ogg>"                  "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:ogg>/$<TARGET_FILE_NAME:ogg>"                  ${CMAKE_SOURCE_DIR}/bin
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:vorbis>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbis>/$<TARGET_FILE_NAME:vorbis>"            "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbis>/$<TARGET_FILE_NAME:vorbis>"            ${CMAKE_SOURCE_DIR}/bin
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:vorbisfile>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbisfile>/$<TARGET_FILE_NAME:vorbisfile>"    "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:vorbisfile>/$<TARGET_FILE_NAME:vorbisfile>"    ${CMAKE_SOURCE_DIR}/bin
    
    # font family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:freetype>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:freetype>/$<TARGET_FILE_NAME:freetype>"        "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:freetype>/$<TARGET_FILE_NAME:freetype>"        ${CMAKE_SOURCE_DIR}/bin
    
    # DirectX family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:DirectX::XAudio2Redist>/$<TARGET_FILE_NAME:DirectX::XAudio2Redist>"        ${CMAKE_SOURCE_DIR}/bin
    
    # fancy family
    
    #COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:fancy2d>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:fancy2d>/$<TARGET_FILE_NAME:fancy2d>"          "$<TARGET_FILE_DIR:LuaSTG>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:fancy2d>/$<TARGET_FILE_NAME:fancy2d>"          ${CMAKE_SOURCE_DIR}/bin
    
    # lua family
    
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:luajit>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            "$<TARGET_FILE_DIR:LuaSTG>"
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:luajit>/$<TARGET_FILE_NAME:luajit>"            ${CMAKE_SOURCE_DIR}/bin
    
    # lua extensions
    
    #COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:lcjson>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lcjson>/$<TARGET_FILE_NAME:lcjson>"            "$<TARGET_FILE_DIR:LuaSTG>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lcjson>/$<TARGET_FILE_NAME:lcjson>"            ${CMAKE_SOURCE_DIR}/bin
    
    #COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:lfs>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lfs>/$<TARGET_FILE_NAME:lfs>"                  "$<TARGET_FILE_DIR:LuaSTG>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lfs>/$<TARGET_FILE_NAME:lfs>"                  ${CMAKE_SOURCE_DIR}/bin
    
    #COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:lcsv>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lcsv>/$<TARGET_FILE_NAME:lcsv>"                "$<TARGET_FILE_DIR:LuaSTG>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lcsv>/$<TARGET_FILE_NAME:lcsv>"                ${CMAKE_SOURCE_DIR}/bin
    
    #COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:lxlsx>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lxlsx>/$<TARGET_FILE_NAME:lxlsx>"              "$<TARGET_FILE_DIR:LuaSTG>"
    #COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:lxlsx>/$<TARGET_FILE_NAME:lxlsx>"              ${CMAKE_SOURCE_DIR}/bin
    
    # luastg engine
    
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:LuaSTG>
    COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE_DIR:LuaSTG>/$<TARGET_FILE_NAME:LuaSTG>"            ${CMAKE_SOURCE_DIR}/bin
)
