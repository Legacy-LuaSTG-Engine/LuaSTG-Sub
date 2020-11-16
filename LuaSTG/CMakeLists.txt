cmake_minimum_required(VERSION 3.15)

set(LUASTG_RESDIR res)
#set(LUASTG_RESDIR custom/THEIS)
#set(LUASTG_RESDIR custom/THTUO)

#set(LUASTG_RESDIR custom/THBDB.develop)
#set(LUASTG_RESDIR custom/THBDB.steam.demo)

add_executable(LuaSTG WIN32)

target_compile_options(LuaSTG PUBLIC
    "/utf-8"
    "/MP4"
    "/std:c++17"
)

target_include_directories(LuaSTG PRIVATE
    src
    ${LUASTG_RESDIR}
    src/LuaWrapper
    
    ../fancylib
    ../fancy2d
    
    ../eyes2d
    
    ../prebuild/luajit/include
    ../prebuild/xmath/include
    ../prebuild/freetype/include
    ../prebuild/dxsdk/include
    
    ../imgui/imgui
    ../imgui/imgui/backends
    ../imgui/imgui/misc/cpp
    ../imgui/imgui/misc/freetype
    
    ../SteamAPI/public/steam
)

target_compile_definitions(LuaSTG PRIVATE
    UNICODE
    _UNICODE
    LUA_BUILD_AS_DLL
    
    #IMGUI_USE_BGRA_PACKED_COLOR
    IMGUI_USE_WCHAR32
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    DLL_IMPORT # freetype
)

target_compile_definitions(LuaSTG PRIVATE LDEVVERSION)

aux_source_directory(src LUASTG_SRC)
aux_source_directory(src/LuaWrapper LUASTG_SRC_LW)
aux_source_directory(src/LuaWrapper/imgui LUASTG_SRC_LW_IMGUI)
aux_source_directory(${LUASTG_RESDIR} LUASTG_RES)

target_sources(LuaSTG PRIVATE
    ${LUASTG_SRC}
    ${LUASTG_SRC_LW}
    ${LUASTG_SRC_LW_IMGUI}
    ${LUASTG_RES}
    
    ${LUASTG_RESDIR}/resource.rc
    ${LUASTG_RESDIR}/app.ico
    ${LUASTG_RESDIR}/splash.png
)

target_link_directories(LuaSTG PRIVATE
    ../prebuild/dxsdk/lib
    ../prebuild/luajit/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/xmath/lib/${EXT_LIBRARY_TYPE}
    ../SteamAPI/redistributable_bin
    
    ../prebuild/zlib/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libzip/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libogg/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/libvorbis/lib/${EXT_LIBRARY_TYPE}
    ../prebuild/freetype/lib/${EXT_LIBRARY_TYPE}
)

target_link_libraries(LuaSTG
    fancylib
    fancy2d
    eyes2d
    
    Gdiplus.lib
    d3d9.lib
    
    freetype.lib
    lua51.lib
    xmath.lib
    
    imgui
)

add_custom_command(TARGET LuaSTG POST_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/copy_bin.bat ${EXT_LIBRARY_TYPE}
)
